/*
 * Copyright 2005-2018 Edscott Wilson Garcia 
 * license: GPL v.3
 */

#define STRUCTURE_CC
#define PARSER "parse7.pl"
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
# undef TRACE
# define TRACE(...)   { (void)0; }
//# define TRACE(...)  fprintf(stderr, "TRACE> "); fprintf(stderr, __VA_ARGS__);
# undef DBG
//# define DBG(...)   { (void)0; }
# define DBG(...)  {fprintf(stderr, "DBG> "); fprintf(stderr, __VA_ARGS__);}
# undef ERROR
# define ERROR(...)  {fprintf(stderr, "*** ERROR> "); fprintf(stderr, __VA_ARGS__);}
# undef WARN
# define WARN(...)  {fprintf(stderr, "warning> "); fprintf(stderr, __VA_ARGS__);}

#include <sys/wait.h>
gchar line[2048];

//static GtkIconTheme *icon_theme=NULL;
GdkPixbuf *focusPixbuf;

enum {
    ICON,
    ICON2,
    TYPEDEF_NAME,
    PROPERTY_NAME,
    PROPERTY_VALUE,
    PROPERTY_SOURCE,
    TYPETAG_INHERITS,
    COLUMNS
};

GMarkupParseContext *mainContext;
GMarkupParseContext *typetagContext;
GMarkupParseContext *propertyContext;
GMarkupParseContext *fileContext;


GtkTreeIter typeTagParent;
GtkTreeIter propertyParent;
GtkTreeIter fileParent;
GtkTreeIter *filesParent;
GtkTreePath *tpathParent;
GtkTreeRowReference *referenceParent;
GtkTreeIter fileChild;
GtkTreeIter *tmpParent=NULL;

FILE *input;

GtkTreeStore *treeStore;
gint dialogMinW_, dialogNatW_, dialogMinH_, dialogNatH_;
GtkWindow *mainWindow;

GtkRequisition minimumSize_;
GtkRequisition naturalSize_;
GtkRequisition maximumSize_;

static void exitApp (GtkButton *widget,
           gpointer   user_data){
    gtk_widget_hide(GTK_WIDGET(mainWindow));
    while (gtk_events_pending()) gtk_main_iteration();
    gtk_main_quit();
    _exit(123);
    return;
}

static gboolean delete_event (GtkWidget *widget,
           GdkEvent  *event,
           gpointer   user_data){
    gtk_widget_hide(widget);
    while (gtk_events_pending()) gtk_main_iteration();
    gtk_main_quit();
    _exit(123);
    return TRUE;
}

    void setWindowMaxSize(void){
	gint x_return, y_return;
	guint w_return, h_return, d_return, border_return;
	Window root_return;
	auto drawable = gdk_x11_get_default_root_xwindow ();
	//Visual Xvisual = gdk_x11_visual_get_xvisual(gdk_visual_get_system());
	auto display = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	XGetGeometry(display, drawable, &root_return,
		&x_return, &y_return, 
		&w_return, &h_return, 
		&border_return, 
		&d_return);
	GdkGeometry geometry;
	geometry.max_width = w_return - 25;
	geometry.max_height = h_return -25;
        maximumSize_.width = geometry.max_width;
        maximumSize_.height = geometry.max_height;
	gtk_window_set_geometry_hints (GTK_WINDOW(mainWindow), GTK_WIDGET(mainWindow), &geometry, GDK_HINT_MAX_SIZE);
    }
    void setDefaultSize(void){
        gtk_widget_get_preferred_size (GTK_WIDGET(mainWindow),
                               &minimumSize_,
                               &naturalSize_);
        setWindowMaxSize();
        TRACE("Size: minimum=%d,%d, natural=%d,%d, max=%d,%d\n",
                minimumSize_.width, minimumSize_.height,
                naturalSize_.width, naturalSize_.height,
                maximumSize_.width, maximumSize_.height);
        gtk_window_set_default_size(mainWindow, 1000, 600);
    }
    
    static GtkTreeViewColumn * 
    mkColumn(void){
        auto column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_resizable(column, FALSE);
        gtk_tree_view_column_set_reorderable(column, TRUE);
        gtk_tree_view_column_set_spacing(column,2);
        return column;
        
    }

    static void
    appendColumnText(GtkTreeView *treeView, const gchar *title, gint columnID){
        auto column = mkColumn();
        /*g_object_set (G_OBJECT (cell), "editable",TRUE,NULL); */
        auto cell = gtk_cell_renderer_text_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, 
                        "text", columnID, 
                        NULL);
        //gtk_tree_view_column_pack_start (column, GtkCellRenderer *cell, FALSE);
        gtk_tree_view_insert_column (treeView,column,-1);
	if (title) gtk_tree_view_column_set_title(column,title);

    }

    static void
    appendColumnPixbuf(GtkTreeView *treeView, gint columnID){
        auto column = mkColumn();
        /*g_object_set (G_OBJECT (cell), "editable",TRUE,NULL); */
        auto cell = gtk_cell_renderer_pixbuf_new();
        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(column, cell, 
                        "pixbuf", columnID, 
                        NULL);
        gtk_tree_view_insert_column (treeView,column,-1);

    }


static GtkWindow *
createWindow(void){
    mainWindow = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    g_signal_connect (G_OBJECT (mainWindow), "delete-event", G_CALLBACK (delete_event), NULL);
    gtk_widget_get_preferred_width (GTK_WIDGET(mainWindow), &dialogMinW_, &dialogNatW_);
    gtk_widget_get_preferred_height (GTK_WIDGET(mainWindow), &dialogMinH_, &dialogNatH_);
    gtk_window_set_type_hint(mainWindow, GDK_WINDOW_TYPE_HINT_DIALOG);
    //setWindowMaxSize(mainWindow);
    gtk_window_set_position (mainWindow, GTK_WIN_POS_MOUSE);

    auto vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 1));
    gtk_container_add (GTK_CONTAINER(mainWindow), GTK_WIDGET(vbox));
    auto topbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
    auto bottombox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1));
    gtk_box_pack_start(vbox, GTK_WIDGET(topbox), FALSE, FALSE, 3);
    gtk_box_pack_start(vbox, GTK_WIDGET(bottombox), TRUE, TRUE, 3);
    auto label = GTK_LABEL(gtk_label_new(""));
    gtk_label_set_markup(label, "<span size=\"larger\" color=\"blue\">Structure</span>");
    gtk_box_pack_start(topbox, GTK_WIDGET(label), FALSE, FALSE, 3);
    auto exit = gtk_button_new_with_label("Exit");
    gtk_box_pack_end(topbox, GTK_WIDGET(exit), FALSE, FALSE, 3);
    g_signal_connect (G_OBJECT (exit), "clicked", G_CALLBACK (exitApp), NULL);

    auto treeView = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(treeStore)));
    //gtk_tree_view_set_show_expanders(treeView, TRUE);
    gtk_tree_view_set_headers_visible(treeView, TRUE);
        appendColumnPixbuf(treeView, ICON);
        appendColumnPixbuf(treeView, ICON2);
        appendColumnText(treeView, "TypeTag/property/value", TYPEDEF_NAME);

//        appendColumnText(treeView, "Property",PROPERTY_NAME );
//        appendColumnText(treeView, "Value", PROPERTY_VALUE);
        appendColumnText(treeView, "Source", PROPERTY_SOURCE);
        appendColumnText(treeView, "inherits", TYPETAG_INHERITS);

    auto scrolledWindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
    gtk_scrolled_window_set_policy(scrolledWindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(bottombox, GTK_WIDGET(scrolledWindow), TRUE, TRUE, 3);

    gtk_container_add(GTK_CONTAINER(scrolledWindow), GTK_WIDGET(treeView));


    setDefaultSize();
    gtk_window_present (mainWindow);
    while (gtk_events_pending()) gtk_main_iteration();



    return mainWindow;

}

static void
startProperty (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    auto parent = (GtkTreeIter *)data; 

    TRACE ("start -> %s\n",element_name); 
    if(strcmp (element_name, "property")!= 0 ){
        fprintf(stderr, "strcmp (element_name, \"property\")!= 0 (%s)\n", element_name);
        return;
    }
    // Simple property into allProperties
    const gchar **name = attribute_names;
    const gchar **value = attribute_values;
    gboolean validValueIter = FALSE;
    for (; name && *name; name++, value++){
        if (strcmp(*name, "name")==0){
            TRACE("Property %s=%s\n", *name, *value); 
        } else {
            TRACE( "         %s=%s\n", *name, *value); 
        }
        GtkTreeIter nameiter;
        GtkTreeIter valueiter;
        GtkTreeIter sourceiter;
        if (strcmp(*name, "name")==0) {
            if (data) {
                gtk_tree_store_append(treeStore, &nameiter, (GtkTreeIter *)data);
            } else {
                gtk_tree_store_append(treeStore, &nameiter, tmpParent);
            }
            auto g = g_strdup_printf("   %s", *value);
            gtk_tree_store_set(treeStore, &nameiter,
               TYPEDEF_NAME, g, -1);
            g_free(g);
        }
        if (strcmp(*name, "value")==0) {
            gtk_tree_store_append(treeStore, &valueiter, &nameiter);
            validValueIter = TRUE;
            auto g = g_strdup_printf("       %s", *value);
            gtk_tree_store_set(treeStore, &valueiter,
               TYPEDEF_NAME, g, -1);
            g_free(g);
        }
        if (validValueIter && strcmp(*name, "source")==0) {
            //gtk_tree_store_append(treeStore, &sourceiter, &valueiter);
            auto g = g_strdup_printf("%s", *value);
            //gtk_tree_store_set(treeStore, &sourceiter,
            gtk_tree_store_set(treeStore, &valueiter,
               PROPERTY_SOURCE, g, -1);
            g_free(g);
        }

    }
    return;
}
static GdkPixbuf *
getEmblem(gchar which){
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();	
    GError *error = NULL;
    const gchar *iconName;
    switch (which) {
        case '1':
            iconName = "emblem-important";
            break;
//        default :
  //          iconName = "emblem-default";
        case '2':
            iconName = "emblem-default";
            break;
        case '3':
            iconName = "emblem-default-symbolic";
            break;
        case '4':
            iconName = "emblem-ok-symbolic";
            break;
        default:
            iconName = "emblem-new";
    }
    TRACE("focus iconName=%s\n", iconName);
    auto pixbuf = gtk_icon_theme_load_icon (icon_theme,
                  iconName,
                  16, 
                  GTK_ICON_LOOKUP_FORCE_SIZE,  // GtkIconLookupFlags flags,
                  &error);
    if (error) {
        ERROR("icons.hh:get_theme_pixbuf: %s\n", error->message);
        g_error_free(error);
        //return 1;
    }
    g_object_ref(pixbuf);
    return pixbuf;
}

static void
startTypeTag (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    TRACE ("start -> %s\n",element_name); 
    if(strcmp (element_name, "typetag")!= 0 ){
        fprintf(stderr, "strcmp (element_name, \"typetag\")!= 0 (%s)\n", element_name);
        return;
    }

    const gchar **name = attribute_names;
    const gchar **value = attribute_values;
    auto parent = (GtkTreeIter *)data; 
    static GtkTreeIter iter;

    for (; name && *name; name++, value++){
        if (strcmp(*name, "name")==0){
            gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, TYPEDEF_NAME, *value, -1);
            TRACE( "TypeTag %s=%s\n", *name, *value); 
        } 
        if (strcmp(*name, "inherits")==0){
            //gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, TYPETAG_INHERITS, *value, -1);
            TRACE( "TypeTag %s=%s\n", *name, *value); 
        }
        if (strcmp(*name, "source")==0){
            //gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, PROPERTY_SOURCE, *value, -1);
            TRACE( "TypeTag %s=%s\n", *name, *value); 
        }
        if (strcmp(*name, "focus")==0){
            //gtk_tree_store_append(treeStore, &iter, parent);
            gtk_tree_store_set(treeStore, &iter, ICON2, getEmblem((*value)[0]), -1);
            TRACE( "focus %s=%s\n", element_name, *value); 
        }
   }
    if (tmpParent) gtk_tree_iter_free(tmpParent);
    tmpParent = gtk_tree_iter_copy(&iter);
    return;
}
int recurseCount = 0;
static void
startFiles(GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    TRACE ("start -> %s\n",element_name); 
            
    if(strcmp (element_name, "files")!= 0 ){
        fprintf(stderr, "strcmp (element_name, \"files\")!= 0 (%s)\n", element_name);
        return;
    }
    recurseCount++;

    
    const gchar **name = attribute_names;
    const gchar **value = attribute_values;
    
    TRACE ("%d: %s %s\n",recurseCount, element_name, *value); 
    for (; name && *name; name++, value++){
        GtkTreeIter parent;
        GtkTreeIter sourceiter;
        if (strcmp(*name, "name")==0) {
            //gtk_tree_store_append(treeStore, &fileChild, filesParent);
            GtkTreePath *tpath = gtk_tree_row_reference_get_path(referenceParent);
            gtk_tree_model_get_iter(GTK_TREE_MODEL(treeStore), &parent, tpath);
            gtk_tree_store_append(treeStore, &fileChild, &parent);
            gtk_tree_path_free(tpath);
            tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(treeStore), &fileChild);
            gtk_tree_row_reference_free(referenceParent);
            referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
            auto g = g_strdup("");
            for (auto i=0;i<recurseCount; i++){
                auto gg = g_strconcat(g, "   ", NULL);
                g_free(g);
                g=gg;
            }
            auto k = g_strdup_printf("%s%s", g, *value);
            gtk_tree_store_set(treeStore, &fileChild,
               TYPEDEF_NAME, k, -1);
               //PROPERTY_NAME, *value, -1);
            g_free(g);
            g_free(k);
        }
        
    }

    return;
}

static void
mainStart (GMarkupParseContext * context,
               const gchar * element_name,
               const gchar ** attribute_names, 
	       const gchar ** attribute_values, 
	       gpointer data, 
	       GError ** error) 
{
    const gchar *name = NULL;
    const gchar *icon = NULL;
    TRACE ("start -> %s\n",element_name); 

    if(strcmp (element_name, "property")==0 ){
        // Simple one line elements:
        startProperty (propertyContext,
               element_name,
               attribute_names, 
	       attribute_values, 
	       &propertyParent, 
	       error);
        return;
    }
    if(strcmp (element_name, "typetag")==0 ){
        startTypeTag (typetagContext,
               element_name,
               attribute_names, 
	       attribute_values, 
	       &typeTagParent, 
	       error);
        // Next lines:
        while(!feof (input)
                && 
                fgets (line, 2048, input) 
                &&
                !strstr(line,"</typetag>"))
        {
            // Simple one line elements:
            line[2048] = 0;
            TRACE("start->\n%s<-end\n", line);
            g_markup_parse_context_parse (propertyContext, line, strlen(line), error);
        }
        return;
    }
    return;
}

GMarkupParser mainParser = {
    mainStart,
    NULL, // mainEnd,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser typeTagParser = {
    startTypeTag,
    NULL,   //endTypeTag,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser propertyParser = {
    startProperty,
    NULL,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};

GMarkupParser fileParser = {
    startFiles,
    NULL,
    NULL,                   /*text_fun, */
    NULL,
    NULL
};


static void
parseXML (const gchar * file) {
    GError *error = NULL;

    TRACE("glib_parser(icon-module): parsing %s\n", file);

    mainContext = g_markup_parse_context_new (&mainParser, (GMarkupParseFlags)0, NULL, NULL);
    typetagContext = g_markup_parse_context_new (&typeTagParser, (GMarkupParseFlags)0, &typeTagParent, NULL);
    propertyContext = g_markup_parse_context_new (&propertyParser, (GMarkupParseFlags)0, NULL, NULL);
    fileContext = g_markup_parse_context_new (&fileParser, (GMarkupParseFlags)0, GINT_TO_POINTER(1), NULL);

    input = fopen (file, "r");
    if(!input) {
        DBG ("cannot open %s\n", file);
        return;
    }
    while(!feof (input) && fgets (line, 2048, input)) {
        line[2048] = 0;
        if (strstr(line, "<files name=") || strstr(line, "</files")){
            if (strstr(line, "<files name=")){
                g_markup_parse_context_parse (fileContext, line, strlen(line), &error);
            } else {
                auto tpath = gtk_tree_row_reference_get_path(referenceParent);
                gtk_tree_path_up(tpath);
                gtk_tree_row_reference_free(referenceParent);
                referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
                recurseCount--;
           }
                    
        } else {
            g_markup_parse_context_parse (mainContext, line, strlen(line), &error);
        }
    }
    fclose (input);

    g_markup_parse_context_free (typetagContext);
    g_markup_parse_context_free (propertyContext);
    g_markup_parse_context_free (fileContext);
    g_markup_parse_context_free (mainContext);
}


int
main (int argc, char *argv[]) {
 /*   DBG("argv[0]=%s\n", argv[0]);
    auto dirname = g_path_get_dirname(argv[0]);
    auto parser = g_strconcat(dirname, G_DIR_SEPARATOR_S, PARSER, NULL);
        char *arg[16];
        int i=0;
        arg[i++] =  g_strdup(parser);
        arg[i++] = argv[1];
        if (argv[2]) arg[i++] = argv[2];
        argv[i] = NULL;
        DBG("child process: %s %s \"%s\"\n", arg[0], arg[1], arg[2]); 
    auto pid = fork();
    if (pid){
        int status;
        DBG("Waiting for process %d\n", pid);
        wait(&status);
        DBG("Wait for process %d complete\n", pid);
    } else {
        execvp(arg[0], arg);
        _exit(123);
    }*/
    //exit(1);

    treeStore = gtk_tree_store_new(COLUMNS, 
	    GDK_TYPE_PIXBUF, // icon in treeView display
	    GDK_TYPE_PIXBUF, // icon in treeView display
	    G_TYPE_STRING,   // typedef name or all-properties 
	    G_TYPE_STRING,   // property name
	    G_TYPE_STRING,   // property value
	    G_TYPE_STRING,   // property source
	    G_TYPE_STRING);   // property typetag
    gtk_tree_store_append(treeStore, &fileParent, NULL);
    gtk_tree_store_set(treeStore, &fileParent, TYPEDEF_NAME, "Files", -1);
    auto tpath = gtk_tree_model_get_path(GTK_TREE_MODEL(treeStore), &fileParent);
    referenceParent = gtk_tree_row_reference_new(GTK_TREE_MODEL(treeStore), tpath);
    gtk_tree_path_free(tpath);
    //filesParent = gtk_tree_iter_copy(&fileParent);
    gtk_tree_store_append(treeStore, &propertyParent, NULL);
    gtk_tree_store_set(treeStore, &propertyParent, TYPEDEF_NAME, "Properties", -1);
    gtk_tree_store_append(treeStore, &typeTagParent, NULL);
    gtk_tree_store_set(treeStore, &typeTagParent, TYPEDEF_NAME, "TypeTags", -1);
    gtk_init(&argc, &argv);

    parseXML(argv[1]?argv[1]:"structure.xml");
	
    
    createWindow();
    gtk_widget_show_all(GTK_WIDGET(mainWindow));
    gtk_main();
    return 0;
}
