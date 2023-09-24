int pan_ena_pin = 2;
int pan_dir_pin = 3;
int pan_pul_pin = 4;
int tilt_ena_pin = 7;
int tilt_dir_pin = 8;
int tilt_pul_pin = 9;

const int pulse_delay = 30;

struct motion_cmd {
    int axis, direction, num_pulses;
};

void setup() {
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pan_ena_pin, OUTPUT);
  pinMode(pan_dir_pin, OUTPUT);
  pinMode(pan_pul_pin, OUTPUT);
  pinMode(tilt_ena_pin, OUTPUT);
  pinMode(tilt_dir_pin, OUTPUT);
  pinMode(tilt_pul_pin, OUTPUT);
  digitalWrite(pan_ena_pin, HIGH); 
  digitalWrite(tilt_ena_pin, HIGH); 
}
void drive(int pul_pin, int dir_pin, int dir, int num_pulses) {
  digitalWrite(dir_pin, dir);
  for (int i=0; i < num_pulses; i++) {
    digitalWrite(pul_pin, HIGH);
    delay(pulse_delay);
    digitalWrite(pul_pin, LOW);
    delay(pulse_delay);
  }
}

motion_cmd get_command(void) {
  String command;
  motion_cmd ret_cmd;
  if (Serial.available() > 0) {
    // read the incoming byte:
    command = Serial.readStringUntil('\n');
    command.trim();
    char axis = command[0];
    if (axis == 'p') {
      ret_cmd.axis = pan_pul_pin;
    } else if (axis == 't') {
      ret_cmd.axis = tilt_pul_pin;
    } else {
      ret_cmd.axis = -1;
    }
  }
  return ret_cmd;
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  drive(pan_pul_pin, pan_dir_pin, LOW, 50);
  drive(pan_pul_pin, pan_dir_pin, HIGH, 50);
  digitalWrite(LED_BUILTIN, LOW);
  drive(tilt_pul_pin, tilt_dir_pin, LOW, 50);
  drive(tilt_pul_pin, tilt_dir_pin, HIGH, 50);
}
