#include <iostream>
#include <vector>
#include <utility>
#include <cmath>
#include <set>
#include <queue>
#include <stack>
#include <fstream>
#include <time.h>
#include <windows.h>
#include <iomanip>
#include "cmd_console_tools.h"
using namespace std;

using ll = long long;

#define ep_node 0 //扩展结点
#define pd_node 1 //生成结点
#define flash 2   //动画效果
#define width 3   //八数码长
#define height 3  //八数码宽
#define star_m 0  //初始化star模式
#define menu_m 1  //初始化menu模式
#define m_dist 1  //曼哈顿距离
#define w_block 2 //错误位置的块数
#define h_0 3

int delay_time = 0; //延迟时间
int select_op = 0;  // 选择画搜索树或者画动画过程
int select_h = 0;   //选择的启发式函数
int step = 0;       //扩展步数
int pd_step = 0;
const int digit = 8;
const int mov[4][2] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}}; //上，右，下，左
const char *pattern = "      ";                           //输出的填充图案
char menu_list[][128] = {
    "欢迎进入八数码问题！",
    "请输入初始状态的八数码图：",
    "请输入终止状态的八数码图：",
    "请选择需要显示的动画：",
    "1.搜索树",
    "2.求解过程",
    "动画延时：（1-3 1为最慢，3为最快）",
    "选择启发式函数h(n)：1.曼哈顿距离 2.位于错误位置的块数 3.h(n)=0"};
// 存储状态信息
struct board
{
public:
    int matrix[3][3];    // 数码存储矩阵
    int index[digit][2]; // 1-8 数码的坐标，用来算h(n)的
    int gfunc;           // g(n)
    int hfunc;           // h(n)
    ll hash_code;        // 用来表示状态的哈希码，保证状态不同哈希码不同，计算方式后面有
    int r0, c0;          // 空白的位置
    board(int m[][3]);

    void init(int m[][3]);

    void cal_hfunc(const board &goal_state);

    void cal(); // 根据matrix计算index,hash_code,r0,c0

    bool operator<(const board &b) const // 重载运算符使优先队列能够根据f(n)=h(n)+g(n)排列
    {
        return (gfunc + hfunc) > (b.gfunc + b.hfunc);
    }

    bool operator==(const board &b) const // 可以直接根据哈希码来判断是否处于同一状态
    {
        return hash_code == b.hash_code;
    }

    void output(const int type) const; // 打印信息的函数
};

stack<board> f;

void board::output(const int type) const
{
    int row, col;        //光标的xy坐标
    cct_getxy(row, col); //获取当前光标的xy坐标
    if (type == pd_node) //如果是生成结点
    {
        row += 4;
        col -= 3 * 3;
    }
    else if (type == ep_node) //如果是扩展结点
    {
        row = 0;
        col++;
    }

    cct_gotoxy(row, col);
    cout << "h(n):" << setw(4) << hfunc << "  g(n):" << setw(4) << gfunc << endl;
    col++;
    cct_gotoxy(row, col);
    int row_0 = row; //保存基点坐标
    int col_0 = col;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (!matrix[i][j]) //如果是空白格
                cct_setcolor(COLOR_HWHITE, COLOR_HWHITE);
            else if (type == ep_node) //扩展和生成结点字体颜色不同
                cct_setcolor(matrix[i][j], COLOR_HWHITE);
            else
                cct_setcolor(matrix[i][j], COLOR_BLACK);
            for (int k = 0; k < 3; k++)
            {
                cct_gotoxy(row, col + k);   // 定位至第k行
                if (k == 1 && matrix[i][j]) // 如果是中间行且非空白格
                    cout << "  " << matrix[i][j] << "   ";
                else
                    cout << pattern;
            }
            row += 6; // 定位至下一个数码
        }
        col_0 += 3;  // 基点坐标下移
        row = row_0; // row col回到基点坐标
        col = col_0;
    }
    cct_setcolor();
}

board::board(int m[][3])
{
    hash_code = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            matrix[i][j] = m[i][j];
            hash_code = hash_code * 10 + m[i][j];
            if (matrix[i][j])
            {
                index[matrix[i][j] - 1][0] = i;
                index[matrix[i][j] - 1][1] = j;
            }
            else
            {
                r0 = i;
                c0 = j;
            }
        }
}

void board::init(int m[][3])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            matrix[i][j] = m[i][j];
            hash_code = hash_code * 10 + m[i][j];
            if (matrix[i][j])
            {
                index[matrix[i][j] - 1][0] = i;
                index[matrix[i][j] - 1][1] = j;
            }
            else
            {
                r0 = i;
                c0 = j;
            }
        }
}

void board::cal_hfunc(const board &goal_state)
{
    hfunc = 0;
    if (select_h == m_dist)
    {
        for (int i = 0; i < digit; i++)
            hfunc += abs(goal_state.index[i][0] - this->index[i][0]) + abs(goal_state.index[i][1] - this->index[i][1]);
    }
    else if (select_h == w_block)
    {
        for (int i = 0; i < digit; i++)
            if (goal_state.index[i][0] != this->index[i][0] || goal_state.index[i][1] != this->index[i][1])
                hfunc++;
    }
    else if (select_h = h_0)
        hfunc = 0;
}

void board::cal()
{
    hash_code = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            hash_code = hash_code * 10 + matrix[i][j]; // 计算状态哈希码
            if (matrix[i][j] == 0)
            {
                r0 = i;
                c0 = j;
                continue;
            }
            index[matrix[i][j] - 1][0] = i;
            index[matrix[i][j] - 1][1] = j;
        }
}

void swap(int &a, int &b)
{
    int temp = a;
    a = b;
    b = temp;
}

vector<vector<board>> info; // board[][]

void A_star(board start_state, board goal_state)
{
    priority_queue<board> q;
    set<ll> s; // 标记已经在队列中或者扩展完成的节点
    start_state.cal_hfunc(goal_state);
    start_state.gfunc = 0;
    q.push(start_state);
    s.insert(start_state.hash_code);
    cout << "初始状态为:";
    start_state.output(ep_node);
    while (!q.empty())
    {
        vector<board> t;
        board temp = q.top();
        f.push(temp);
        q.pop();
        step++; //扩展结点数+1
        t.push_back(temp);
#if 0
        cout << endl
             << endl
             << endl;
        cout << "将要扩展";
        temp.output(ep_node);
#endif
        if (temp == goal_state)
        {
            info.push_back(t);
            return;
        }
        int tr0, tc0;

        for (int i = 0; i < 4; i++)
        {
            tr0 = temp.r0 + mov[i][0];
            tc0 = temp.c0 + mov[i][1];

            if (tr0 < 0 || tr0 > 2 || tc0 < 0 || tc0 > 2)
                continue;
            board nxt_state = temp;
            swap(nxt_state.matrix[temp.r0][temp.c0], nxt_state.matrix[tr0][tc0]);
            nxt_state.cal();
            nxt_state.cal_hfunc(goal_state);
            nxt_state.gfunc = temp.gfunc + 1;
            if (s.find(nxt_state.hash_code) == s.end()) // 该节点没有访问过
            {
                pd_step++;
                t.push_back(nxt_state);
#if 0
                nxt_state.output(pd_node);
#endif
                q.push(nxt_state);
                s.insert(nxt_state.hash_code);
            }
        }
        info.push_back(t);
    }

    return;
}

int cal_reverse(int array[width * height - 1]) //计算逆序对
{
    int len = width * height - 1;
    int cnt = 0;
    for (int i = 0; i < len - 1; i++)
        for (int j = i + 1; j < len; j++)
            if (array[i] > array[j])
                cnt++;
    return cnt;
}

bool judge(int start[][height], int goal[][height])
{
    int array1[width * height - 1] = {}, array2[width * height - 1] = {};
    for (int i = 0, k = 0; i < width; i++) //将二维数组放入一维数组中
        for (int j = 0; j < height; j++)
        {
            if (start[i][j] != 0)
            {
                array1[k] = start[i][j];
                k++;
            }
        }
    for (int i = 0, k = 0; i < width; i++)
        for (int j = 0; j < height; j++)
        {
            if (goal[i][j] != 0)
            {
                array2[k] = goal[i][j];
                k++;
            }
        }
    int cnt1 = cal_reverse(array1);
    int cnt2 = cal_reverse(array2);
    return cnt1 % 2 == cnt2 % 2; //逆序对个数相同 问题有解
}

void init(int mode)
{
    cct_cls(); //清屏
    cct_setcolor();
    cct_setconsoleborder(120, 120, 120, 9000); //设置控制台(缓冲区)大小
    cct_setcursor(CURSOR_VISIBLE_NORMAL);
    if (mode == star_m)
        cct_setfontsize("新宋体", 16, 8); //设置字体
    else if (mode == menu_m)
        cct_setfontsize("新宋体", 24, 16); //设置字体
}

void output_T(const board &b)
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            cout << b.matrix[i][j] << ' ';
        cout << endl;
    }
}

void draw_SearchTree()
{
    cct_cls();
    for (auto it = info.begin(); it != info.end(); it++)
    {
        cout << "将要扩展";
        (*it)[0].output(ep_node);
        int len = it->size();
        for (int i = 1; i < len; i++)
            (*it)[i].output(pd_node);
        cout << endl
             << endl
             << endl;
    }
}

int cal_h_distance(const board &t1, const board &t2)
{
    int hdistance = 0;
    for (int i = 0; i < digit; i++)
        hdistance += abs(t1.index[i][0] - t2.index[i][0]) + abs(t1.index[i][1] - t2.index[i][1]);
    return hdistance;
}

void draw_Animation()
{
    cct_cls();
    int pos_x = 0, pos_y = 0;
    cct_getxy(pos_x, pos_y);
    stack<board> s;
    board before = f.top();
    f.pop();
    s.push(before);
    while (!f.empty())
    {
        board now = f.top();
        f.pop();
        int t1 = before.gfunc - now.gfunc;
        if (t1 == 1)
        {
            int t2 = cal_h_distance(before, now);
            if (t2 == 1)
            {
                s.push(now);
                before = now;
            }
        }
    }
    do
    {
        board temp = s.top();
        s.pop();
        cct_gotoxy(pos_x, pos_y);
        temp.output(flash);
        Sleep(delay_time);
        getchar();
    } while (!s.empty());
}

int menu(int start[][height], int goal[][height])
{
    init(menu_m);
    cout << menu_list[0] << endl;
    cout << menu_list[1] << endl;
    for (int i = 0; i < width; i++) //输入八数码
        for (int j = 0; j < height; j++)
            cin >> start[i][j];
    cout << menu_list[2] << endl;
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            cin >> goal[i][j];
    int res = judge(start, goal); //判断是否有解
    if (!res)
        return -1;
    cout << menu_list[7] << endl;
    cin >> select_h;
    if (cin.fail() || select_h < 1 || select_h > 3)
    {
        cin.clear();
        select_h = m_dist; //缺省为曼哈顿距离
    }
    cout << menu_list[3] << endl;
    cout << menu_list[4] << "   " << menu_list[5] << endl;
    cin >> select_op;
    if (cin.fail() || select_op <= 1 || select_op > 2)
    {
        cin.clear();
        select_op = 1; // 默认画搜索树
    }

    else
    {
        int time_op = 0;
        cout << menu_list[6] << endl;
        cin >> time_op; //输入时间选项
        if (cin.fail() || time_op < 1 || time_op > 3)
        {
            cin.clear();
            delay_time = 800;
        }
        else if (time_op == 1)
            delay_time = 1200;
        else if (time_op == 2)
            delay_time = 800;
        else
            delay_time = 400;
    }
    return 1;
}

int main()
{
    int start[width][height] = {};
    int goal[width][height] = {};
    // int start[3][3] = {{2, 8, 3}, {1, 6, 4}, {7, 0, 5}};
    // int goal[3][3] = {{1, 2, 3}, {8, 0, 4}, {7, 6, 5}};
    int res = menu(start, goal);
    if (res == -1)
    {
        cout << "无法从初始状态抵达最终状态" << endl;
        system("pause");
        return -1;
    }
    init(star_m);
    LARGE_INTEGER tick, begin, end;
    QueryPerformanceFrequency(&tick); //获取计数器频率
    QueryPerformanceCounter(&begin);  //获取初始记录
    A_star(board(start), board(goal));
    QueryPerformanceCounter(&end); //获取终止记录
    // cout << endl
    //<< endl;
    // cout << "已达到目标状态！" << endl;
    // cout << "完整动画效果为：" << endl;
    // cout << endl;
    switch (select_op)
    {
    case 1:
        draw_SearchTree();
        break;
    case 2:
        draw_Animation();
        break;
    }
    cout << endl;
    cout << "扩展结点数为：" << step << endl;
    cout << "生成节点数为：" << pd_step << endl;
    cout << "搜索时间为：";
    cout << setiosflags(ios::fixed) << setprecision(6) << double(end.QuadPart - begin.QuadPart) / tick.QuadPart << "秒" << endl;
    system("pause");
    return 0;
}