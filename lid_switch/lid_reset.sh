print_usage() {
  echo "usage: ./lid_reset.sh -d <just flag to disable lid switch>"
  exit 1
}


systemctl_reset() {
    systemctl restart systemd-logind.service
}


enable_lid_switch() {
    sudo cp logind_original.conf /etc/systemd/logind.conf
    systemctl_reset
}

disable_lid_switch() {
    sudo cp logind_disabled.conf /etc/systemd/logind.conf
    systemctl_reset
}


no_args="true"
while getopts "de" flag; do
  case ${flag} in
    d) disable_lid_switch
        ;;
    e) enable_lid_switch
        ;;
    *) print_usage
      exit 1 ;;
  esac
  no_args="false"
done

if [ "$no_args" == "true" ]; then
  print_usage
fi

