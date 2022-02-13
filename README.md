# buddhabrot
c++ buddhabrot renderer

For all platforms, just compile normally with g++ (I don't think MSVC supports srand, sorry to MVS 2019 users)
I'm on windows, so the executable and batch file are both there if you want it

Credit to the article I read by Melinda Green: https://superliminal.com/fractals/bbrot/
Inigo Quilez sent me down the buddhabrot rabbithole: https://iquilezles.org/index.html/

Large speedup is by using main cardioid and period 3 bulb detection
I can still optimize this a bunch, but I'm too lazy
Sorry about messy inefficient code, I'm not very good at using c++
