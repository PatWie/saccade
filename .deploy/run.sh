sudo docker run --user="$(id -u):$(id -g)" --net=none -v /tmp/saccade:/data -i -t patwie/saccade-static .deploy/pipeline.sh
