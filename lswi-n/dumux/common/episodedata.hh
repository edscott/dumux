// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   See the file COPYING for full copying permissions.                      *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
#ifndef EPISODE_DATA_HH
#define EPISODE_DATA_HH
     
#include <dune/common/exceptions.hh>
/*!
 * \file
 * \ingroup Common
 * \brief Base class template for generalized input parameters, stages and episodes.
 */
namespace Dumux {
struct episodeData {
    using Scalar = double;
public:
    std::string name_;
    Scalar *stageData_;
    int stageNumber_;
    Scalar lowerTimeStepBoundary_;
    Scalar upperTimeStepBoundary_;
};

template <class TypeTag>
class EpisodeData {
    using Scalar = GetPropType<TypeTag, Properties::Scalar>;
    using GridView = GetPropType<TypeTag, Properties::GridView>;
    using GlobalPosition = Dune::FieldVector<Scalar, GridView::dimensionworld>;



    enum { nonPhaseComponents = 1 };

    typedef std::map<const char *, int> Map;
    Map scalarMap_;

protected:

    int episodes_;
    int stages_;
    GlobalPosition upperRight_;
    std::string name_;
    Scalar TEnd_;

    
private:
    Scalar *problemArray_;
    episodeData *episodeArray_;
    Scalar **stageArray_;
    std::string *stageName_;


public:
    
    EpisodeData(void): 
        episodes_(0), 
        stages_(0),
        problemArray_(nullptr),
        episodeArray_(nullptr),
        stageArray_(nullptr),
        stageName_(nullptr)
    {}

    ~EpisodeData(void){
        if (problemArray_) free(problemArray_);
        if (episodeArray_) free(episodeArray_);
        if (stageArray_) {
            for (int i=0; i<stages_; i++) {if (stageArray_[i]) free(stageArray_[i]);}
            free(stageArray_);
        }
        if (stageName_) free(stageName_);
    }

    // Construct parameter arrays for initial conditions, stages,
    // and episodes.
    void init(const char **scalars) {
        auto count = getProblemData(scalars);
        TEnd_ = getParam<int>("TimeLoop.TEnd");
        stages_ = getParam<int>("Problem.Stages", 0);
        if (stages_) {
            getStageData(count, scalars);
            getEpisodeData();
        }
    }
protected:

    // Construct group name for stage i.
    std::string stageGroup(int i){
        std::string stage = "Stage." + std::to_string(i+1);
        return stage;
    }

    int stageNumber(int k) const {
        return (episodeArray_+k)->stageNumber_;
    }


    // Get amount of episodes in stage i.
    int getStageEpisodes(int i){
        std::string stage = stageGroup(i);
        int stageEpisodes = 1;
        if (hasParam(stage + ".Episodes")) stageEpisodes = getParam<int>(stage + ".Episodes");
        return stageEpisodes;
    }

    // Return stage i name. 
    std::string stageName(int i){
        return stageName_[i];
    }

    // Return episode i name.
    std::string episodeName(int i){
        return (episodeArray_ + i)->name_;
    }

    // Return episode parameter id from global episode number.
    std::string episodeGroup(int episodeIdx){
        int i = 0;
        int j = 0;
        int k = 0;
        for (i=0; i<stages_; i++){
          for (j=0; j<episodes_; j++, k++){
              if (k == episodeIdx){
                  //TRACE("*** found at stage %d\n", i);
                  return episodeGroup(i,j);
              }
          }
        }
        std::string message = "Episode ";
        message += std::to_string(episodeIdx);
        message += "not found";
        return message;
    }

    // Return episode parameter id from stage number and local episode number.
    std::string episodeGroup(int i, int j){
        std::string episode = stageGroup(i) + "." + std::to_string(j+1);
        return episode;
    }

    // Set stage parameter value from runtime code.
    bool set(const char *idx, Scalar value) const {
        auto it = scalarMap_.find(idx);
        if (it == scalarMap_.end()) {
            std::string p = idx;
            DUNE_THROW(Dumux::ParameterException,  "getValue(): parameter "<< p << " not in map");
        }
        problemArray_[it->second]=value;
        return true;
    }

    // Return problem parameter value from parameter id.
    Scalar getValue(const char *idx) const {
        auto it = scalarMap_.find(idx);
        if (it == scalarMap_.end()) {
            std::string p = idx;
            DUNE_THROW(Dumux::ParameterException,  "getValue(): parameter "<< p << " not in map");
        }
        return problemArray_[it->second];
    }

    // Return stage parameter value from stage index and parameter id.
    bool setStageValue(int stage, const char *idx, Scalar value){
        auto it = scalarMap_.find(idx);
        if (it == scalarMap_.end()) {
            std::string p = idx;
            DUNE_THROW(Dumux::ParameterException,  "getStageValue(): parameter "<< p << " not in map");
        }
        stageArray_[stage][it->second] = value;
        return true;
    }

    // Return stage parameter value from stage index and parameter id.
    Scalar getStageValue(int stage, const char *idx){
        auto it = scalarMap_.find(idx);
        if (it == scalarMap_.end()) {
            std::string p = idx;
            DUNE_THROW(Dumux::ParameterException,  "getStageValue(): parameter "<< p << " not in map");
        }
        return stageArray_[stage][it->second];
    }


    // Return episode parameter value from global episode index and parameter id.
    Scalar getEpisodeValue(int episode, const char *idx){
        auto it = scalarMap_.find(idx);
        if (it == scalarMap_.end()) {
            std::string p = idx;
            DUNE_THROW(Dumux::ParameterException,  "getEpisodeValue(): parameter "<< p << " not in map");
        }
        return episodeArray_[episode].stageData_[it->second];
    }

    // Set stage parameter value from runtime code.
    bool set(int episode, const char *parameter, Scalar value) const{
        if (episodes_ == 0) return false;
        
        episodeData  *ed = episodeArray_+episode;
        auto it = scalarMap_.find( parameter );
        if (it == scalarMap_.end()){
            std::string p = parameter;
            std::cerr << "episodedata.hh: "<<parameter<<" notfound.\n";
            return false;
        }
        ed->stageData_[it->second] = value;
    }

    // Return stage parameter value from global episode index and parameter id.
    Scalar get(int episode, const char *parameter) const{
        if (episodes_ == 0){
            return getValue(parameter);
        }
        episodeData  *ed = episodeArray_+episode;
        if (strcmp(parameter,"lowerTimeStepBoundary")==0) 
            return ed->lowerTimeStepBoundary_;
        if (strcmp(parameter,"upperTimeStepBoundary")==0) 
            return ed->upperTimeStepBoundary_;
        auto it = scalarMap_.find( parameter );
        if (it == scalarMap_.end()){
            std::string p = parameter;
            DUNE_THROW(Dumux::ParameterException,  "get(ed): parameter "<< p << " not found");
        }
        return ed->stageData_[it->second];
    }

    // Return stage parameter value from global episode index and parameter id.
    Scalar getFromStage(int stage, const char *parameter) const{
        auto it = scalarMap_.find( parameter );
        if (it == scalarMap_.end()){
            std::string p = parameter;
            DUNE_THROW(Dumux::ParameterException,  "get(ed): parameter "<< p << " not found");
        }
        return stageArray_[stage][it->second];
    }

    // Kaput function for out of memory error.
    void 
    callocError(const char *s){
        std::string message = s;
        DUNE_THROW(Dune::OutOfMemoryError, message << strerror(errno) << "\n");
        
    }


private:

    // Construct map from parameter id to array index.
    static int fillMap(const char **scalars, Map& map){
        int count=0;
        for (auto p=scalars; p && *p; p++) {
            const auto [it, success] = map.insert({*p, count});
            count++;
        }
        return count;
    }
    
    // Read top level parameter from input.
    static Scalar getS(const char *parameter){
        std::string problem = "Problem.";
        std::string spatial = "SpatialParams.";
        std::string timeloop = "TimeLoop.";
        std::string variable;

        if (hasParam(problem + parameter)) variable = problem + parameter;
        else if (hasParam(spatial + parameter)) variable = spatial + parameter;
        else if (hasParam(timeloop + parameter)) variable = timeloop + parameter;
        else {
            std::string p = parameter;
            DUNE_THROW(Dumux::ParameterException,  "Fix this: variable Problem." << p <<
                    " not found.");
        }
        
        auto value = getParam<Scalar>(variable);
        
        return value;
    }

    // Read parameter for specific group from input, with default value.
    static Scalar getS(std::string group, const char *parameter, Scalar *defaultvalue){
        std::string variable = group;
        variable += ".";
        variable += parameter;
        Scalar value;
        if (defaultvalue) value = getParam<Scalar>(variable, *defaultvalue);
        else value = getParam<Scalar>(variable);     
        return value;
    }
 
    // Fill in problem data array from input.
    int getProblemData(const char **scalars){
        int count = fillMap(scalars, scalarMap_);

        problemArray_ = (Scalar *)calloc(count, sizeof(Scalar));
        if (!problemArray_)callocError("Data constructor");

        int i=0;
        for (auto p=scalars; p && *p; p++,i++) {
            problemArray_[i] = getS(*p);
        }
        name_   = getParam<std::string>("Problem.Name");
        // Grid data
        upperRight_  = getParam<GlobalPosition>("Grid.UpperRight");
        return count;
    }

    // Fill in stage data arrays from input.
    void getStageData(int count, const char **scalars){
        // Stages info:
        stageName_ = (std::string *)calloc(stages_, sizeof(std::string));
        if (!stageName_)callocError("Data constructor stageName_");
        stageArray_ = (Scalar **)calloc(stages_, sizeof(Scalar *));
        if (!stageArray_)callocError("Data constructor stageArray_");

        for (int i=0; i<stages_; i++){
            stageArray_[i] = (Scalar *)calloc(count, sizeof(Scalar));
            if (!stageArray_[i])callocError("Data constructor stageArray_[i]");
            episodes_ += getStageEpisodes(i);
            int j=0;
            std::string stage = stageGroup(i);
            for (auto p=scalars; p && *p; p++,j++) {
                stageArray_[i][j] = getS(stage, *p, problemArray_+j);
            }
            
            std::string name = stage + ".Name";
            if (hasParam(name)){
                stageName_[i] = getParam<std::string>(name);
            } else {
                stageName_[i] = stage;
            }


        } 
    }

    // Fill in episode data arrays from input.
    void getEpisodeData(void){
        episodeArray_ = (episodeData *)calloc(episodes_, sizeof(episodeData));
        if (!episodeArray_)callocError("Data constructor episodeArray_");
        int k=0;
        for (int i=0; i<stages_; i++){
            std::string stage = stageGroup(i);
            auto stageEpisodes = getStageEpisodes(i);
            for (int j=0; j<stageEpisodes; j++, k++){
                (episodeArray_+k)->stageNumber_ = i;
                std::string episode = episodeGroup(i,j);
                //TRACE("getEpisodeData %s...\n", episode.c_str());
                std::string name = episode + ".Name";
                if (hasParam(name)){
                    (episodeArray_+k)->name_ = getParam<std::string>(name);
                } else {
                    (episodeArray_+k)->name_ = episode;
                }

                (episodeArray_+k)->stageData_ = stageArray_[i];
                (episodeArray_+k)->lowerTimeStepBoundary_ = 
                    getParam<Scalar>(episode+".lowerTimeStepBoundary");
                (episodeArray_+k)->upperTimeStepBoundary_ = 
                    getParam<Scalar>(episode+".upperTimeStepBoundary");
            }
        }
    }

};

    

}
#endif
