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
#define private public

/*
 *   Universe - 
 *
 *
 *
 */
namespace game_state {
    Map::Map(){
        int partial_map_fd=-1;
        snprintf(path, sizeof(path), PARTIALMAP_FILE_PATH_FORMAT, next_partial_map_id++);
        PartialMap empty_partial_map[w*h];
        memset(empty_partial_map, NULL, sizeof(PartialMap)*w*h);
        if((partial_map_fd = open(path, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) != -1) close(partial_map_fd);
        else if((partial_map_fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) != -1){
            //for(int i=w*h; i; --i) 
            //for(int j=0; j<PARTIAL_MAP_BYTES_LENGTH; ++j)
            //assert(sizeof(PartialMap) == write(partial_map_fd, &ZERO_FILL, sizeof(PartialMap)));
            write(partial_map_fd, empty_partial_map, w*h*sizeof(PartialMap));
            close(partial_map_fd);
            //partial_map_file = fopen(path, "rb+");
        }
        else perror("Error: "); //printf("error while create and write metadata in file %s\n", path);
    }
    int Map::getPartialMap(PartialMap* partial_map, int x, int y){
        assert(x < w && x >= 0);
        assert(y < h && y >= 0);
        int partial_map_fd=-1;
        if((partial_map_fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR)) != -1){
            assert(sizeof(PartialMap) == pread(partial_map_fd, partial_map, sizeof(PartialMap), (y*w + x)*sizeof(PartialMap)));
            close(partial_map_fd);
        }
        else perror();
        //fseek(partial_map_file, (y*w + x)*sizeof(PartialMap), SEEK_SET);
        //assert(sizeof(PartialMap) == fread(partial_map, 1, sizeof(PartialMap), partial_map_file));
    }
    int Map::updatePartialMap(PartialMap* partial_map, int x, int y){
        assert(x < w && x >= 0);
        assert(y < h && y >= 0);
        int partial_map_fd=-1;
        if((partial_map_fd = open(path, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) != -1){ 
            close(partial_map_fd);
        }
        if((partial_map_fd = open(path, O_WRONLY, S_IRUSR | S_IWUSR)) != -1){
            assert(sizeof(PartialMap) == pwrite(partial_map_fd, partial_map, sizeof(PartialMap), (y*w + x)*sizeof(PartialMap)));
            close(partial_map_fd);
        }
        else perror();
        //fseek(partial_map_file, (y*w + x)*sizeof(PartialMap), SEEK_SET);
        //assert(sizeof(PartialMap) == fwrite(partial_map, 1, sizeof(PartialMap), partial_map_file));
    }
    int Map::next_partial_map_id  = 0;

    Planet::Planet() {
        m = tool::rand(1, 40);
        if(m > 10) type = JOVIAN;
        else type = TERRESTIRAL;
    }

    int Universe::save(const char* path){
        if(FILE* f = fopen(path, "wb")){ 
            assert(sizeof(*this) == fwrite(this, 1, sizeof(*this), f));
            fclose(f);
            return 0;
        }
        return 1;
        //assert(sizeof(children_num) == fwrite(&children_num, 1, sizeof(child_num), f));
    }
    int Universe::load(const char* path){
        if(FILE* f = fopen(path, "rb")){ 
            assert(sizeof(*this) == fread(this, 1, sizeof(*this), f));
            fclose(f);
            return 0;
        }
        return 1;
        //assert(sizeof(children_num) == fwrite(&children_num, 1, sizeof(child_num), f));
    }


};

