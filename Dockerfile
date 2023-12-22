from ubuntu:18.04

RUN apt update
RUN apt install -y software-properties-common
RUN add-apt-repository ppa:eidelen/d2m
RUN apt update
RUN apt install -y dicom2mesh

ENTRYPOINT ["dicom2mesh"]
# use like so:
# docker run -v `pwd`/my/dicom/files/:/data dicom2mesh -i /data -t 557 -o /data/mesh.stl
# your mesh will be alongside your dicom images
