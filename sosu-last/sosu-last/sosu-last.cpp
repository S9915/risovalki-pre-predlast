#include <windows.h>
#include <math.h>
#include <tchar.h>
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR WinName[] = _T("MainFrame");

int APIENTRY WinMain(HINSTANCE This, HINSTANCE Prev, LPSTR cmd, int mode)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wc;

	wc.hInstance = This;
	wc.lpszClassName = WinName;
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	if (!RegisterClass(&wc)) return 0;
	hWnd = CreateWindow(WinName, _T("Каркас Windows-приложения"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		HWND_DESKTOP,
		NULL,
		This,
		NULL);
	ShowWindow(hWnd, mode);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	} return 0;
}

float v11, v12, v13, v21, v22, v23, v31, v32, v33, v43;//
float rho = 300., thetta = 75., phi = 30., ScreenDist = 500.;//
float th, ph, cosph, costh, sinth, sinph;
float factor = atan(1.0) / 45.;//

void VidMatCoeff(float rho, float thetta, float phi)
{
	//Определение компонентов направления наблюдения для сведения с
//лучом освещения
	th = thetta * factor; ph = phi * factor;
	costh = cos(th); sinth = sin(th); cosph = cos(ph); sinph = sin(ph);
	//Элементы матрицы V
	v11 = -sinth; v12 = -cosph * costh; v13 = -sinph * costh;
	v21 = costh; v22 = -cosph * sinth; v23 = -sinph * sinth;
	v31 = 0.; v32 = sinph; v33 = -cosph;
	v43 = rho;
}

POINT Perspective(float x, float y, float z)
{
	POINT point;
	float xe, ye, ze;
	VidMatCoeff(rho, thetta, phi);
	xe = v11 * x + v21 * y;
	ye = v12 * x + v22 * y + v32 * z;
	ze = v13 * x + v23 * y + v33 * z + v43;
	//Экранные координаты
	point.x = ScreenDist * xe / ze + 400.;
	point.y = ScreenDist * ye / ze + 300.;
	return point;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	float A, B, C, D, An, Bn, Cn;//
	float xt[3], yt[3], zt[3];//
	float Al, Bl, Cl;
	float alpha;
	const int WIDTH = 400;
	const int HEIGHT = 300;
	POINT pointl[4];
	HDC hdc;
	int sx, sy, xPos, yPos, zDelta;
	PAINTSTRUCT ps;
	static HBRUSH hBrush;

	class TFPoint
	{
	public:
		float X;
		float Y;
		float Z;
	};
	//Задаем координаты точек куба
	TFPoint CubePoints[] =
	{
		{-50,-50,-50},
		{50,-50,-50},
		{50,50,-50},
		{-50,50,-50},
		{-50,50,50},
		{-50,-50,50},
		{50,-50,50},
		{50,50,50}
	};
	int Gran[6][4] =
	{
		{0,3,4,5},
		{0,5,6,1},
		{2,7,4,3},
		{7,6,5,4},
		{0,1,2,3},
		{2,1,6,7}
	};
	//--------------------------------------------------------

		 switch (message)
		 {

		 case WM_MOUSEMOVE:
			 sx = LOWORD(lParam);//координата мыши по оси X
			 sy = HIWORD(lParam);//координата мыши по оси Y
			 thetta += ((sx % 180) - 90) / 10;
			 phi += ((sy % 180) - 90) / 10;
			 InvalidateRect(hWnd, NULL, TRUE);
			 break;

		 case WM_MOUSEWHEEL:
			 zDelta = (int)wParam;//wheel rotation
			 ScreenDist -= zDelta / 1000000.;
			 InvalidateRect(hWnd, NULL, TRUE);
			 break;

		 case WM_PAINT:
			 hdc = BeginPaint(hWnd, &ps);
			 th = thetta * factor; ph = phi * factor;
			 costh = cos(th); sinth = sin(th); cosph = cos(ph); sinph = sin(ph);
			 A = rho * sinph * costh; B = rho * sinph * sinth; C = rho * cosph;
			 Al = A / (sqrt(A * A + B * B + C * C));
			 Bl = B / (sqrt(A * A + B * B + C * C));
			 Cl = C / (sqrt(A * A + B * B + C * C));
			 for (int i = 0; i < 6; i++)
			 { //
				 for (int j = 0; j < 3; j++)
				 {
					 xt[j] = CubePoints[Gran[i][j]].X;
					 yt[j] = CubePoints[Gran[i][j]].Y;
					 zt[j] = CubePoints[Gran[i][j]].Z;
				 }
				 //
				 A = yt[0] * (zt[1] - zt[2]) - yt[1] * (zt[0] - zt[2]) + yt[2] * (zt[0] - zt[1]);
				 B = -(xt[0] * (zt[1] - zt[2]) - xt[1] * (zt[0] - zt[2]) + xt[2] * (zt[0] - zt[1]));
				 C = xt[0] * (yt[1] - yt[2]) - xt[1] * (yt[0] - yt[2]) + xt[2] * (yt[0] - yt[1]);
				 //
				 An = A / (sqrt(A * A + B * B + C * C));
				 Bn = B / (sqrt(A * A + B * B + C * C));
				 Cn = C / (sqrt(A * A + B * B + C * C));
				 //
				 alpha = (An * Al + Bn * Bl + Cn * Cl);
				 //
				 for (int j = 0; j < 4; j++)
				 {
					 pointl[j] = Perspective(CubePoints[Gran[i][j]].X,
						 CubePoints[Gran[i][j]].Y,
						 CubePoints[Gran[i][j]].Z);
				 }
				 //Определение направления обхода точек грани для выделения ориентации к
			   //наблюдателю
				 D = pointl[0].x * (pointl[1].y - pointl[2].y) -
					 pointl[1].x * (pointl[0].y - pointl[2].y) +
					 pointl[2].x * (pointl[0].y - pointl[1].y);
				 if (D < 0)
				 {
					 hBrush = CreateSolidBrush(RGB((1 - alpha) * 255,
						 (1 - alpha) * 255, (1 - alpha) * 255));
					 SelectObject(hdc, hBrush);
					 Polygon(hdc, pointl, 4);
				 }
			 }
			 EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}