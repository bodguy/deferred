#ifndef TIME_H
#define TIME_H

class Time {
public:
  Time();

  void Update();
  float GetDeltaTime() const;
  float ElapsedTime() const;

private:
  float deltaTime, lastFrame;
};

#endif //TIME_H
