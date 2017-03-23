#include <Servo.h>
#define NPINS 3
#define FOR_EVERY_PIN for (int i = 0; i < NPINS; i++ )

#define POS_ONE 1
#define POS_TWO 2
#define POS_THREE 3
#define POS_FOUR 4
#define POS_FIVE 5
#define POS_NULL 0

#define MAX_AGITATION 100
#define MIN_AGITATION 10
#define MIN_SERVO_PERIOD 1
#define MAX_SERVO_PERIOD 30
#define SERVO_PERIOD_NOISE 3
#define SHADOW_CHANGE_THRESHOLD 5


static int PIN_IDS[3] = {A0, A1, A2};
static int photoArray[3];
static int baseline[3];
static int sigma[3] = {SHADOW_CHANGE_THRESHOLD, SHADOW_CHANGE_THRESHOLD, SHADOW_CHANGE_THRESHOLD};
static bool shadow[3] = {false, false, false};
static int handPos;
static int handVel;

static int framesSinceLastUpdate = 0;
static int framesBetweenUpdate = 100;

static int agitation = MIN_AGITATION;

// TODO: class these
Servo servoA;
static int servoAPin = 3;
static int servoAPos = 0;
static int servoAVel = 1;
static int servoAFramesSinceLastMove = 0;
static int servoAFramesPerMove = MAX_SERVO_PERIOD;

Servo servoB;
static int servoBPin = 5;
static int servoBPos = 0;
static int servoBVel = 1;
static int servoBFramesSinceLastMove = 0;
static int servoBFramesPerMove = MAX_SERVO_PERIOD;


void setup() {
    //implicit input on PHOTO_0-2

    // init baseline
    FOR_EVERY_PIN {
        baseline[i] = analogRead(PIN_IDS[i]);
    }

    delay(10);
    servoA.attach(servoAPin);
    servoB.attach(servoBPin);

    Serial.begin(9600);
}

/* utils */

void printArray(int* arr) {
    FOR_EVERY_PIN {
        Serial.print(arr[i]);
        Serial.print(' ');
    }
    Serial.println();
}

void printShadowArray() {
    FOR_EVERY_PIN {
        if (shadow[i]) {
          Serial.print('X');

        } else {
          Serial.print('_');
        }
        Serial.print(' ');
    }
    Serial.println();
}

/* refresh state */
void updatePhotoArray() {
    FOR_EVERY_PIN {
        photoArray[i] = analogRead(PIN_IDS[i]);
    }
}

void updateShadow() {
    FOR_EVERY_PIN {
        if (photoArray[i] < (baseline[i] - sigma[i])) {
            shadow[i] = true;
        } else {
            shadow[i] = false;
        }
    }
}

int getHandPos() {
    FOR_EVERY_PIN {
        if (shadow[i]) {
            // see if hand is between two
            if (i < (NPINS - 1) && shadow[i+1]) {
                if (i == 0) {return POS_TWO; }
                else if (i==1) {return POS_FOUR;}
            }

            // hand isn't hitting two pins. must be over single pin.
            if (i == 0) {return POS_ONE; }
            if (i == 1) {return POS_THREE; }
            if (i == 2) {return POS_FIVE; }
        }
    }
    return POS_NULL;
}

void updateHand() {
    int newHandPos = getHandPos();
    handVel = newHandPos - handPos;
    handPos = newHandPos;
}

bool anyHand() {
  FOR_EVERY_PIN {
    if (shadow[i]) {
      return true;
    }
  }
  return false;
}

void updateAgitation() {


    if (handVel == 1) {
        agitation -= 10;
    } else if (handVel == 2 || handVel == 3) {
        agitation -= 6;
    }
    if (anyHand()) {
      agitation += 1;
    } else {
      agitation += 3;
    }
    agitation = constrain(agitation, MIN_AGITATION, MAX_AGITATION);
}


void updateServoSpeeds() {
    int newServoPeriod = map(agitation, MIN_AGITATION, MAX_AGITATION, MAX_SERVO_PERIOD, MIN_SERVO_PERIOD);

    servoAFramesPerMove = constrain(
        random(newServoPeriod - SERVO_PERIOD_NOISE, newServoPeriod + SERVO_PERIOD_NOISE),
        MIN_SERVO_PERIOD, MAX_SERVO_PERIOD);
    servoBFramesPerMove = constrain(
        random(newServoPeriod - SERVO_PERIOD_NOISE, newServoPeriod + SERVO_PERIOD_NOISE),
        MIN_SERVO_PERIOD, MAX_SERVO_PERIOD);
}
void loop() {
    updateServoSpeeds();


    framesSinceLastUpdate += 1;
    if (framesSinceLastUpdate >= framesBetweenUpdate) {
        updatePhotoArray();
        updateShadow();
        updateHand();
        updateAgitation();
        framesSinceLastUpdate = 0;

        Serial.print("baseline: ");
        printArray(baseline);
        Serial.print("photo: ");
        printArray(photoArray);
        Serial.print("shadow: ");
        printShadowArray();
        Serial.print("agitation: ");
        Serial.println(agitation);
        Serial.println();
    }

    servoAFramesSinceLastMove += 1;
    if (servoAFramesSinceLastMove >= servoAFramesPerMove) {
        servoAPos = constrain(servoAPos + servoAVel, 0, 180);
        if (servoAPos == 0 || servoAPos == 180) servoAVel = -servoAVel;
        servoA.write(servoAPos);
        servoAFramesSinceLastMove = 0;
    }

    servoBFramesSinceLastMove += 1;
    if (servoBFramesSinceLastMove >= servoBFramesPerMove) {
        servoBPos = constrain(servoBPos + servoBVel, 0, 180);
        if (servoBPos == 0 || servoBPos == 180) servoBVel = -servoBVel;
        servoB.write(servoBPos);
        servoBFramesSinceLastMove = 0;
    }
    delay(1000 / 360);
}
