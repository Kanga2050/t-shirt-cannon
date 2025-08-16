#include <Servo.h>

const uint8_t ri[] = {2, 3, 4, 5};
const uint8_t f1i = 6, f2i = 7, hni = 8;
const uint8_t jl = 9, jr = 10;
const uint8_t rf1 = 11, rf2 = 12, rhn = A0, rjp = A1, rtc = A2, rta = A3, rwu = A4, rwd = A5;
const uint8_t ro[] = {rf1, rf2, rhn, rjp, rtc, rta, rwu, rwd};

const uint16_t pmin = 1000, pmax = 2000, pmid = 1500;
const uint32_t fms = 200, flk = 1500, tmo = 100000;

volatile uint16_t pw[4] = {pmid, pmid, pmid, pmid};
volatile uint32_t rt[4], lt[4];
volatile uint8_t ps;

Servo sl, sr;
uint32_t ft[2];
bool fp[2], sv[2], ok;

ISR(PCINT2_vect) {
  uint32_t t = micros();
  uint8_t s = PIND;
  uint8_t c = s ^ ps;
  ps = s;
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t b = 1 << ri[i];
    if (!(c & b)) continue;
    if (s & b) {
      rt[i] = t;
    } else {
      uint32_t d = t - rt[i];
      if (d > 800 && d < 2200) {
        pw[i] = d;
        lt[i] = t;
      }
    }
  }
}

int lv(uint8_t i) {
  noInterrupts();
  uint16_t p = pw[i];
  interrupts();
  int v = map(constrain(p, pmin, pmax), pmin, pmax, -100, 100);
  return abs(v) < 4 ? 0 : v;
}

bool live() {
  uint32_t t = micros();
  noInterrupts();
  uint32_t a = lt[0], b = lt[1];
  interrupts();
  return t - a < tmo && t - b < tmo;
}

void rl(uint8_t p, bool on) {
  digitalWrite(p, on ? LOW : HIGH);
}

void off() {
  sl.writeMicroseconds(pmid);
  sr.writeMicroseconds(pmid);
  for (uint8_t p : ro) rl(p, false);
  sv[0] = sv[1] = false;
}

void sh(uint8_t i, bool in, uint8_t p, uint32_t t) {
  if (in && !fp[i] && t - ft[i] > flk) {
    ft[i] = t;
    sv[i] = true;
  }
  if (sv[i] && t - ft[i] >= fms) sv[i] = false;
  fp[i] = in;
  rl(p, sv[i]);
}

void setup() {
  for (uint8_t p : ro) {
    digitalWrite(p, HIGH);
    pinMode(p, OUTPUT);
  }
  for (uint8_t p : ri) pinMode(p, INPUT);
  pinMode(f1i, INPUT);
  pinMode(f2i, INPUT);
  pinMode(hni, INPUT);
  sl.attach(jl);
  sr.attach(jr);
  off();
  ps = PIND;
  PCICR |= 1 << PCIE2;
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20) | (1 << PCINT21);
}

void loop() {
  uint32_t t = millis();
  if (!live()) {
    off();
    ok = false;
    return;
  }
  bool a = digitalRead(f1i), b = digitalRead(f2i);
  if (!ok) {
    if (a || b) {
      off();
      return;
    }
    ok = true;
    fp[0] = fp[1] = false;
  }

  rl(rjp, true);
  sl.writeMicroseconds(pmid + lv(0) * 5);
  sr.writeMicroseconds(pmid + lv(1) * 5);

  int tt = lv(2), wn = lv(3);
  rl(rtc, tt > 50);
  rl(rta, tt < -50);
  rl(rwu, wn > 50);
  rl(rwd, wn < -50);
  rl(rhn, digitalRead(hni));

  sh(0, a, rf1, t);
  sh(1, b, rf2, t);
}
