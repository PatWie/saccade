sudo docker run --user="$(id -u):$(id -g)" --net=none -v /tmp/saccade:/data -i -t qt59static .deploy/pipeline.sh
