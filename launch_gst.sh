#! /bin/bash


dt=$(date '+%Y%m%d_%H%M%S')
cwd=$(pwd)

launch_pipe=(
	"gst-launch-1.0 tcambin tcam-properties=tcam,ExposureAuto=On,serial=46810510"
	" ! video/x-raw,format=BGRx,framerate=60/1,width=1920,height=1080 ! timeoverlay "
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
