#define _USE_MATH_DEFINES
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <thread>
#include <easyx.h>
#include <graphics.h>
#include <chrono>
#include <cmath>
#include <mutex>

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

using namespace std;
using ld = long double;
using ull = unsigned long long;

constexpr ull itPrec = 100000; //迭代精度
ull cpPrec = 200; //计算每1^2方格的边长分值
constexpr ld vFs = 2.0; //判定发散值
constexpr ull mColorR = 114, mColorG = 51, mColorB = 4; //颜色值乘的
constexpr ull dColorR = 0, dColorG = 0, dColorB = 0; //颜色值加的
constexpr ull ctrlM = 2; //鼠标点击缩放倍数
ld sx1, sx2, sy1, sy2; //上次渲染的坐标
HWND jiaodian, tJiaodian; //绘图窗口的句柄与当前窗口的句柄
constexpr ull threads = 32; // 增加线程数量以提高渲染速度
mutex mtx;

void draw_pixel(ld re, ld im, ull tcl) {
    lock_guard<mutex> lock(mtx);
    putpixel(((re - sx1) * cpPrec), ((im - sy1) * cpPrec), RGB(mColorR * tcl + dColorR, mColorG * tcl + dColorG, mColorB * tcl + dColorB));
}

void cp_q(ld x1, ld y1, ld x2, ld y2) {
    sx1 = x1, sx2 = x2, sy1 = y1, sy2 = y2;
    ld delta_c = 1.0 / cpPrec;
    vector<thread> thread_pool;

    for (ull t = 0; t < threads; ++t) {
		thread_pool.emplace_back([=]() { //多线程渲染
            for (ld re = x1 + t * delta_c; re < x2; re += delta_c * threads) {
                for (ld im = y1; im < y2; im += delta_c) {
                    ull tcl = 0;
                    ld rec = 0, imc = 0, re2c = 0, im2c = 0, re_oldc = 0; //快速计算点
					for (ull i = 0; i < itPrec; i++) { //迭代计算
                        rec = re2c - im2c + re; 
                        imc = 2 * re_oldc * imc + im;
                        re_oldc = rec;
                        re2c = rec * rec;
                        im2c = imc * imc;
                        if (sqrt(re2c + im2c) > vFs) {
                            tcl = 200 * i;
                            break;
                        }
                    }
                    draw_pixel(re, im, tcl);
                }
            }
            });
    }

    for (auto& t : thread_pool) {
        t.join();
    }
}

void frm() {
    tJiaodian = GetForegroundWindow();
    POINT p;
    GetCursorPos(&p); //获得鼠标的位置
    ScreenToClient(tJiaodian, &p);
    ld x1, y1, x2, y2;
    x1 = 1.0 * p.x / cpPrec + sx1;
    y1 = 1.0 * p.y / cpPrec + sy1;
	if (KEY_DOWN(VK_LBUTTON) && tJiaodian == jiaodian && p.x > 0 && p.y > 0) { //鼠标left缩放
        cpPrec = cpPrec * ctrlM;
        x2 = x1 + (sx2 - sx1) / ctrlM * 0.5;
        y2 = y1 + (sy2 - sy1) / ctrlM * 0.5;
        x1 = x1 - (sx2 - sx1) / ctrlM * 0.5;
        y1 = y1 - (sy2 - sy1) / ctrlM * 0.5; 
        cout << x1 << " " << y1 << " " << x2 << " " << y2 << "\n精度1/" << cpPrec << "\n";
        cleardevice();
        cp_q(x1, y1, x2, y2);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
	else if (KEY_DOWN(VK_RBUTTON) && tJiaodian == jiaodian && p.x > 0 && p.y > 0) { //鼠标right缩放
        cpPrec = cpPrec / ctrlM;
        x2 = x1 + (sx2 - sx1) * ctrlM * 0.5;
        y2 = y1 + (sy2 - sy1) * ctrlM * 0.5;
        x1 = x1 - (sx2 - sx1) * ctrlM * 0.5;
        y1 = y1 - (sy2 - sy1) * ctrlM * 0.5;
        cout << x1 << " " << y1 << " " << x2 << " " << y2 << "\n精度1/" << cpPrec << "\n";
        cleardevice();
        cp_q(x1, y1, x2, y2);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    this_thread::sleep_for(chrono::milliseconds(50));
}

int main() {
    auto tm1 = chrono::high_resolution_clock::now();
    initgraph(800, 800, EX_SHOWCONSOLE);
    jiaodian = GetForegroundWindow();
	cp_q(-2, -2, 2, 2); //初始化
    auto tm2 = chrono::high_resolution_clock::now();
    cout << "Time taken: " << chrono::duration_cast<chrono::milliseconds>(tm2 - tm1).count() << " ms\n";
    while (true) frm();
    closegraph();
    return 0;
}
