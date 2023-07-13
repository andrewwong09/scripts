#! /bin/bash


dt=$(date '+%Y%m%d_%H%M%S')
cwd=$(pwd)

# DFM 37UX178-ML: Dimensions at 60 fps: 1920,1080 / 2048,2048 / 3072,2048
width=3072
height=2048
fps=60

launch_pipe=(
	"gst-launch-1.0 tcambin tcam-properties=tcam,ExposureAuto=On,serial=46810510"
	" ! video/x-raw,format=BGRx,framerate=$fps/1,width=$width,height=$height ! timeoverlay "
)
launch_pipe=${launch_pipe[*]}

display_pipe="videoconvert ! autovideosink"
save_pipe="videoconvert ! x265enc ! h265parse ! matroskamux ! filesink location=$cwd/$dt.mkv"

record() {
	pipeline="$launch_pipe ! $save_pipe"
}

display() {
	pipeline="$launch_pipe ! $display_pipe"
}

both() {
	pipeline="$launch_pipe ! tee name=t t. ! queue ! $display_pipe t. ! queue ! $save_pipe"
}


while getopts "rdb" flag; do
  case ${flag} in
    r) record
	;;
    d) display
	;;
    b) both
	;;
    *) print_usage
      exit 1 ;;
  esac
done

echo $pipeline
$pipeline
