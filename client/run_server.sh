#/bin/bash

#./build.sh

#build_dir=".build"
build_dir="./"
h2o_conf="h2o.conf"

# start server on builded
sudo h2o --conf $build_dir/$h2o_conf &
echo $! > .pid
