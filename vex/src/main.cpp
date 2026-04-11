#include "vex.h"

using namespace vex;

brain br;
controller ct = controller(primary);

pwm_out dl = pwm_out(br.ThreeWirePort.A);
pwm_out dr = pwm_out(br.ThreeWirePort.B);
pwm_out tt = pwm_out(br.ThreeWirePort.C);
pwm_out wn = pwm_out(br.ThreeWirePort.D);
digital_out fr = digital_out(br.ThreeWirePort.E);
digital_out fl = digital_out(br.ThreeWirePort.F);
digital_out hn = digital_out(br.ThreeWirePort.G);
analog_in pt = analog_in(br.ThreeWirePort.H);

int cl(int v) {
  if (v > 100) return 100;
  if (v < -100) return -100;
  return v;
}

int cu(int v) {
  if (v > -15 && v < 15) return 0;
  double x = v / 100.0;
  return (int)(100 * x * x * x);
}

int tri(bool p, bool n) {
  if (p) return 100;
  if (n) return -100;
  return 0;
}

double psi() {
  double v = pt.value(analogUnits::range12bit) * 5.0 / 4095.0;
  if (v < 0.3) return -1;
  if (v > 4.5) v = 4.5;
  if (v < 0.5) v = 0.5;
  return (v - 0.5) * 75.0;
}

int main() {
  fr.set(false);
  fl.set(false);
  hn.set(false);
  int n = 0;
  while (true) {
    int y = cu(ct.Axis3.position());
    int x = cu(ct.Axis1.position());
    dl.state(cl(y + x), percent);
    dr.state(cl(y - x), percent);
    tt.state(tri(ct.ButtonB.pressing(), ct.ButtonY.pressing()), percent);
    wn.state(tri(ct.ButtonR1.pressing(), ct.ButtonR2.pressing()), percent);

    bool ar = ct.ButtonL2.pressing();
    bool f = ar && ct.ButtonA.pressing();
    fr.set(f);
    fl.set(!f && ct.ButtonX.pressing());
    hn.set(ct.ButtonL1.pressing());

    if (++n % 10 == 0) {
      double p = psi();
      ct.Screen.setCursor(1, 1);
      if (p < 0) ct.Screen.print("no sensor  %s   ", ar ? "ARMED" : "safe");
      else ct.Screen.print("%3.0f psi  %s   ", p, ar ? "ARMED" : "safe");
      br.Screen.clearScreen();
      br.Screen.setCursor(1, 1);
      if (p < 0) br.Screen.print("pressure sensor not read");
      else br.Screen.print("chamber %3.0f psi", p);
    }
    wait(20, msec);
  }
}
