#define _USE_MATH_DEFINES
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <thread>
#include <easyx.h>
#include <graphics.h>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

using namespace std;
using ld = long double;
using ull = unsigned long long;

ull itPrec = 100; //迭代精度
ull cpPrec = 200; //计算每1^2方格的边长分值
ld vFs = 2.0; //判定发散值
ull mColorR = 114, mColorG = 51, mColorB = 4; //颜色值乘的
ull dColorR = 0, dColorG = 0, dColorB = 0; //颜色值加的
ull ctrlM = 2; //鼠标点击缩放倍数
ld sx1, sx2, sy1, sy2; //上次渲染的坐标
HWND jiaodian, tJiaodian; //绘图窗口的句柄与当前窗口的句柄
ull threads = 2; 

ImgInfo imgIf;

void cp_q(ld x1, ld y1, ld x2, ld y2) {
	sx1 = x1, sx2 = x2, sy1 = y1, sy2 = y2;
	ld delta_c = 1.0 / cpPrec;
    //ull it1 = 0, it2 = 0;
	for (ld re = x1; re < x2; re += delta_c) {
        imgIf.imgData.push_back(vector<char>());
		for (ld im = y1; im < y2; im += delta_c) {
			ull tcl = 0;
			ld rec = 0, imc = 0, re2c = 0, im2c = 0, re_oldc = 0; //快速计算点
			for (ull i = 0; i < itPrec; i++) {
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
            //imgIf.imgData[it1].push_back(RGB(mColorR * tcl + dColorR, mColorG * tcl + dColorG, mColorB * tcl + dColorB));
			putpixel(((re - x1) * cpPrec), ((im - y1) * cpPrec), RGB(mColorR * tcl + dColorR, mColorG * tcl + dColorG, mColorB * tcl + dColorB));
            //++it2;
		}
        //++it1;
	}
} 

void rdThread(ld x1, ld y1, ld x2, ld y2) {
	sx1 = x1, sx2 = x2, sy1 = y1, sy2 = y2;
	thread* th = new thread[threads]();
	ld jz = (x2 - x1) / threads;
	for (int i = 0; i < threads; i++) {
		th[i] = thread(cp_q, x1 + i * jz, y1, x1 + i * jz + jz, y2);
		th[i].detach();
	}
	while (1);
}

void frm() {
	tJiaodian = GetForegroundWindow();
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(tJiaodian, &p);
	ld x1, y1, x2, y2;
	x1 = 1.0 * p.x / cpPrec + sx1;
	y1 = 1.0 * p.y / cpPrec + sy1;
	if (KEY_DOWN(VK_LBUTTON) && tJiaodian == jiaodian && p.x>0 && p.y>0) {
		cpPrec = cpPrec * ctrlM;
		x2 = x1 + (sx2 - sx1) / ctrlM * 0.5;
		y2 = y1 + (sy2 - sy1) / ctrlM * 0.5;
		x1 = x1 - (sx2 - sx1) / ctrlM * 0.5;
		y1 = y1 - (sy2 - sy1) / ctrlM * 0.5; 
		cout << x1 << " " << y1 << " " << x2 << " " << y2 << "\n精度1/" << cpPrec << "\n";
		cleardevice();
		cp_q(x1, y1, x2, y2);
		//rdThread(x1, y1, x2, y2);
		Sleep(100);
	}
	else if (KEY_DOWN(VK_RBUTTON) && tJiaodian == jiaodian && p.x > 0 && p.y > 0) {
		cpPrec = cpPrec / ctrlM;
		x2 = x1 + (sx2 - sx1) * ctrlM * 0.5;
		y2 = y1 + (sy2 - sy1) * ctrlM * 0.5;
		x1 = x1 - (sx2 - sx1) * ctrlM * 0.5;
		y1 = y1 - (sy2 - sy1) * ctrlM * 0.5;
		cout << x1 << " " << y1 << " " << x2 << " " << y2 << "\n精度1/" << cpPrec << "\n";
		cleardevice();
		cp_q(x1, y1, x2, y2);
		//rdThread(x1, y1, x2, y2);
		Sleep(100);
	}
    Sleep(50);
}

int main() {
	ios::sync_with_stdio(0);
	int tm1 = clock();
	initgraph(800, 800, EX_SHOWCONSOLE);
	jiaodian = GetForegroundWindow();
    //imgIf = readBitmap("F:\\cpp_imgs\\test1.bmp");
	cp_q(-2, -2, 2, 2);
    //saveBitmap(imgIf);
	cout << clock() - tm1;
	while (1) frm();
	closegraph();
	return 0;
}
