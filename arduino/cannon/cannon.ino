#include <Servo.h>
#include <Wire.h>

const uint8_t ri[] = {2, 3, 4, 5};
const uint8_t fi = 6, li = 7, hi = 8;
const uint8_t jl = 9, jr = 10;
const uint8_t kf = 11, kl = 12, kh = 13, kj = A0, kt1 = A1, kt2 = A2, kw1 = A3, kw2 = 1;
const uint8_t ko[] = {kf, kl, kh, kj, kt1, kt2, kw1, kw2};

const uint16_t pmin = 1000, pmax = 2000, pmid = 1500;
const uint32_t fms = 200, flk = 1500, tmo = 100000, dz = 50;

const uint8_t mx = 0x70, as = 0x36, cht = 0, che = 1;
const int16_t tlim = 150, emin = 0, emax = 60;

volatile uint16_t pw[4] = {pmid, pmid, pmid, pmid};
volatile uint32_t rt[4], lt[4];
volatile uint8_t ps;

Servo sl, sr;
uint32_t ft, et, mt[2];
bool fp, sv, ok, eo[2];
int8_t md[2];
int16_t e0[2], ea[2];

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

bool ch(uint8_t c) {
  Wire.beginTransmission(mx);
  Wire.write(1 << c);
  return Wire.endTransmission() == 0;
}

bool rd(uint8_t c, int16_t &deg) {
  if (!ch(c)) return false;
  Wire.beginTransmission(as);
  Wire.write(0x0C);
  if (Wire.endTransmission(false)) return false;
  if (Wire.requestFrom(as, (uint8_t)2) != 2) return false;
  uint16_t r = ((Wire.read() & 0x0F) << 8) | Wire.read();
  deg = (int32_t)r * 360 / 4096;
  return true;
}

int16_t wrap(int16_t a) {
  while (a > 180) a -= 360;
  while (a < -180) a += 360;
  return a;
}

void enc() {
  for (uint8_t i = 0; i < 2; i++) {
    int16_t d;
    eo[i] = rd(i ? che : cht, d);
    if (eo[i]) ea[i] = wrap(d - e0[i]);
  }
}

void pair(uint8_t i, uint8_t a, uint8_t b, int8_t want, int16_t lo, int16_t hi) {
  if (eo[i] && ((want > 0 && ea[i] >= hi) || (want < 0 && ea[i] <= lo))) want = 0;
  uint32_t t = millis();
  if (want != md[i]) {
    if (md[i] != 0 && want != 0) want = 0;
    md[i] = want;
    mt[i] = t;
  }
  bool go = t - mt[i] > dz;
  rl(a, go && md[i] > 0);
  rl(b, go && md[i] < 0);
}

void off() {
  sl.writeMicroseconds(pmid);
  sr.writeMicroseconds(pmid);
  for (uint8_t p : ko) rl(p, false);
  sv = false;
  md[0] = md[1] = 0;
}

void setup() {
  for (uint8_t p : ko) {
    digitalWrite(p, HIGH);
    pinMode(p, OUTPUT);
  }
  for (uint8_t p : ri) pinMode(p, INPUT);
  pinMode(fi, INPUT);
  pinMode(li, INPUT);
  pinMode(hi, INPUT);
  sl.attach(jl);
  sr.attach(jr);
  off();
  Wire.begin();
  Wire.setWireTimeout(3000, true);
  for (uint8_t i = 0; i < 2; i++) {
    int16_t d;
    if (rd(i ? che : cht, d)) e0[i] = d;
  }
  ps = PIND;
  PCICR |= 1 << PCIE2;
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20) | (1 << PCINT21);
}

void loop() {
  uint32_t t = millis();
  if (t - et > 20) {
    et = t;
    enc();
  }
  if (!live()) {
    off();
    ok = false;
    return;
  }
  bool f = digitalRead(fi), l = digitalRead(li);
  if (!ok) {
    if (f || l) {
      off();
      return;
    }
    ok = true;
    fp = false;
  }

  rl(kj, true);
  sl.writeMicroseconds(pmid + lv(0) * 5);
  sr.writeMicroseconds(pmid + lv(1) * 5);

  int tt = lv(2), wn = lv(3);
  pair(0, kt1, kt2, tt > 50 ? 1 : tt < -50 ? -1 : 0, -tlim, tlim);
  pair(1, kw1, kw2, wn > 50 ? 1 : wn < -50 ? -1 : 0, emin, emax);
  rl(kh, digitalRead(hi));

  if (f && !fp && t - ft > flk) {
    ft = t;
    sv = true;
  }
  if (sv && t - ft >= fms) sv = false;
  fp = f;
  rl(kf, sv);
  rl(kl, l && !sv && t - ft > fms + 300);
}
