#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "game_data.hpp"
#include "config.hpp"
#include "tool.hpp"
//#define private public

/*
 *   Universe - 
 *
 *
 *
 */
namespace game_state {

    class Tile{
        private:
            int x, y, z;
            int owner;
    };
    constexpr const char ZERO_FILL[sizeof(Tile)] = {0,};
    class PartialMap{
        private:
            Tile tiles[PARTIAL_MAP_HEIGHT*PARTIAL_MAP_WIDTH];
    };
    class Land{
        private:
            enum Landform{PLAIN};
            static constexpr const char* PARTIALMAP_FILE_PATH = "data/partial_map.data"; // id
            static int partial_map_index_fd;
            static int partial_map_fd;
            char path[MAX_PATH_LENGTH];
            //int partial_map_id = 0;
            int w=0, h=0;
            int position_in_file=0;
            Landform landform;
        public:
            Land(){ }
            int initialize(int _w, int _h, ...){
                assert(!(w && w*h < _w*_h))
                w = _w; h = _h;
                PartialMap empty_partial_map[w*h];
                memset(empty_partial_map, NULL, sizeof(PartialMap)*w*h);
                position_in_file = lseek(partial_map_fd, 0, SEEK_END);
                write(partial_map_fd, empty_partial_map, w*h*sizeof(PartialMap));
                close(partial_map_fd);
            }
            int getPartialMap(PartialMap* partial_map, int x, int y){
                assert(x < w && x >= 0);
                assert(y < h && y >= 0);
                if((partial_map_fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR)) != -1){
                    assert(sizeof(PartialMap) == pread(partial_map_fd, partial_map, sizeof(PartialMap), position_in_file + (y*w + x)*sizeof(PartialMap)));
                    close(partial_map_fd);
                }
                else perror("Error: ");
                //fseek(partial_map_file, (y*w + x)*sizeof(PartialMap), SEEK_SET);
                //assert(sizeof(PartialMap) == fread(partial_map, 1, sizeof(PartialMap), partial_map_file));
            }
            int updatePartialMap(PartialMap* partial_map, int x, int y){
                assert(x < w && x >= 0);
                assert(y < h && y >= 0);
                int partial_map_fd=-1;
                if((partial_map_fd = open(path, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) != -1){ 
                    close(partial_map_fd);
                }
                if((partial_map_fd = open(path, O_WRONLY, S_IRUSR | S_IWUSR)) != -1){
                    assert(sizeof(PartialMap) == pwrite(partial_map_fd, partial_map, sizeof(PartialMap), position_in_file + (y*w + x)*sizeof(PartialMap)));
                    close(partial_map_fd);
                }
                else perror("Error: ");
                //fseek(partial_map_file, (y*w + x)*sizeof(PartialMap), SEEK_SET);
                //assert(sizeof(PartialMap) == fwrite(partial_map, 1, sizeof(PartialMap), partial_map_file));
            }
            ~Land(){
            }
    };
    int Land::partial_map_index_fd = open(Land::PARTIALMAP_INDEX_FILE_PATH, O_RDWR| O_CREAT, S_IRUSR | S_IWUSR);
    int Land::partial_map_fd = open(Land::PARTIALMAP_FILE_PATH, O_RDWR| O_CREAT, S_IRUSR | S_IWUSR);
    vector<int> Land::partial_map_index;

    class AstroSystem {
        private:
            char name[MAX_NAME_LENGTH] = "";
            float orbital_angle, orbital_radius, mass, radius, orbital_angle_velocity, rotation_angle_velocity, rotation_shaft, orbital_shaft;
            int children_num=0;
            AstroSystem* children;
            //vector<AstroSystem*> children;
        public:
            virtual AstroSystem(){ }
            virtual ~AstroSystem(){
                if(children_num){
                    for(children_num-=1; children_num>=0; --children_num)
                        delete(children[children_num]);
                    delete[](children);
                }
            }
            virtual int save(FILE* f){
                fwrite(this, 1, sizeof(*this), f);
                for(int i=0; i<children_num; ++i)
                    children[i].save(f);
                return 0;
            }
            virtual int save(const char* path){
                if(FILE* f = fopen(path, "wb")){ 
                    save(f);
                    fclose(f);
                    return 0;
                }
                return 1;
            }
            virtual int load(FILE* f){
                if(children_num){
                    for(children_num-=1; children_num>=0; --children_num)
                        delete(children[children_num]);
                    delete[](children);
                }
                fread(this, 1, sizeof(*this), f);
                children = new AstroSystem[children_num];
                for(int i=0; i<children_num; ++i)
                    children[i].load(f);
            }
            virtual int load(const char* path){
                if(FILE* f = fopen(path, "rb")){ 
                    assert(sizeof(*this) == fread(this, 1, sizeof(*this), f));
                    fclose(f);
                    return 0;
                }
                return 1;
            }
    };

    class Satellite : Land, public AstroSystem{
        private:
        public:
            Satellite() {
            }
            int save(FILE* f){
                fwrite(this, 1, sizeof(*this), f);
                for(int i=0; i<children_num; ++i)
                    children[i].save(f);
                return 0;
            }
    };

    class Planet : Land, public AstroSystem{
        private:
            enum Type{JOVIAN, TERRESTIRAL};
            Type type;
        public:
            Planet() {
                m = tool::rand(1, 40);
                if(m > 10) type = JOVIAN;
                else type = TERRESTIRAL;
            }
            int save(FILE* f){
                fwrite(this, 1, sizeof(*this), f);
                for(int i=0; i<children_num; ++i)
                    children[i].save(f);
                return 0;
            }
    };

    class Star : public AstroSystem{
        private:
        public:
            Star(){
            }
    };

    class Galaxy : public AstroSystem{
        private:
        public:
            Galaxy(){
            }
    };

    class Universe : public AstroSystem {
        private:
        public:
            Universe(){
                //std::cout << "hi" << std::endl;
                //strcpy(name, tool::randomSelect<const char*>(game_data::UNIVERSE_NAME_POOL, game_data::UNIVERSE_NAME_POOL_SIZE));
            }
/*
            int save(const char* path){
                if(FILE* f = fopen(path, "wb")){ 
                    assert(sizeof(*this) == fwrite(this, 1, sizeof(*this), f));
                    fclose(f);
                    return 0;
                }
                return 1;
                //assert(sizeof(children_num) == fwrite(&children_num, 1, sizeof(child_num), f));
            }
            int load(const char* path){
                if(FILE* f = fopen(path, "rb")){ 
                    assert(sizeof(*this) == fread(this, 1, sizeof(*this), f));
                    fclose(f);
                    return 0;
                }
                return 1;
                //assert(sizeof(children_num) == fwrite(&children_num, 1, sizeof(child_num), f));
            }
*/
    };


};
