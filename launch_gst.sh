#! /bin/bash


source $HOME/scratch/tiscamera/build/env.sh

dt=$(date '+%Y%m%d_%H%M%S')
cwd=$(pwd)

# DFM 37UX178-ML: Dimensions at 60 fps: 1920,1080 / 2048,2048 / 3072,2048
width=3072
height=2048
fps=60

launch_pipe=(
	"gst-launch-1.0 tcambin tcam-properties=tcam,ExposureAuto=Off,serial=46810510"
	" ! video/x-raw,format=BGRx,framerate=$fps/1,width=$width,height=$height ! timeoverlay"
)
launch_pipe=${launch_pipe[*]}

display_pipe="videoconvert ! autovideosink"
save_pipe="videoconvert ! x265enc ! h265parse ! matroskamux ! filesink location=$cwd/$dt.mkv"
images_pipe="videoconvert ! jpegenc ! multifilesink location=$dt/frame%06d.jpg"

record() {
	pipeline="$launch_pipe ! $save_pipe"
}

display() {
	pipeline="$launch_pipe ! $display_pipe"
}

images() {
	mkdir $dt
	pipeline="$launch_pipe ! $images_pipe"
}

both() {
	pipeline="$launch_pipe ! tee name=t t. ! queue ! $display_pipe t. ! queue ! $save_pipe"
}

print_usage() {
	echo ""
	echo "USAGE----------------------"
	echo "Display: ./launch_gst.sh -d"
	echo "Record : ./launch_gst.sh -r"
	echo "images : ./launch_gst.sh -i"
	echo ""
}

no_args="true"
while getopts "rdib" flag; do
  case ${flag} in
    r) record
	;;
    d) display
	;;
    i) images
	;;
    b) both
	;;
    *) print_usage
      exit 1 ;;
  esac
  no_args="false"
done

[[ $no_args == "true" ]] && { print_usage; exit 1; }

echo $pipeline
$pipeline
