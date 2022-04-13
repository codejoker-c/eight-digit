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

#define ep_node 0 //��չ���
#define pd_node 1 //���ɽ��
#define flash 2   //����Ч��
#define width 3   //�����볤
#define height 3  //�������
#define star_m 0  //��ʼ��starģʽ
#define menu_m 1  //��ʼ��menuģʽ
#define m_dist 1  //�����پ���
#define w_block 2 //����λ�õĿ���

int delay_time = 0; //�ӳ�ʱ��
int select_op = 0;  // ѡ�����������߻���������
int select_h = 0;   //ѡ�������ʽ����
int step = 0;       //��չ����
const int digit = 8;
const int mov[4][2] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}}; //�ϣ��ң��£���
const char *pattern = "      ";                           //��������ͼ��
char menu_list[][128] = {
    "��ӭ������������⣡",
    "�������ʼ״̬�İ�����ͼ��",
    "��������ֹ״̬�İ�����ͼ��",
    "��ѡ����Ҫ��ʾ�Ķ�����",
    "1.������",
    "2.������",
    "������ʱ����1-3 1Ϊ������3Ϊ��죩",
    "ѡ������ʽ����h(n)��1.�����پ��� 2.λ�ڴ���λ�õĿ���"};
// �洢״̬��Ϣ
struct board
{
public:
    int matrix[3][3];    // ����洢����
    int index[digit][2]; // 1-8 ��������꣬������h(n)��
    int gfunc;           // g(n)
    int hfunc;           // h(n)
    ll hash_code;        // ������ʾ״̬�Ĺ�ϣ�룬��֤״̬��ͬ��ϣ�벻ͬ�����㷽ʽ������
    int r0, c0;          // �հ׵�λ��
    board(int m[][3]);

    void init(int m[][3]);

    void cal_hfunc(const board &goal_state);

    void cal(); // ����matrix����index,hash_code,r0,c0

    bool operator<(const board &b) const // ���������ʹ���ȶ����ܹ�����f(n)=h(n)+g(n)����
    {
        return (gfunc + hfunc) > (b.gfunc + b.hfunc);
    }

    bool operator==(const board &b) const // ����ֱ�Ӹ��ݹ�ϣ�����ж��Ƿ���ͬһ״̬
    {
        return hash_code == b.hash_code;
    }

    void output(const int type) const; // ��ӡ��Ϣ�ĺ���
};

stack<board> f;

void board::output(const int type) const
{
    int row, col;        //����xy����
    cct_getxy(row, col); //��ȡ��ǰ����xy����
    if (type == pd_node) //��������ɽ��
    {
        row += 4;
        col -= 3 * 3;
    }
    else if (type == ep_node) //�������չ���
    {
        row = 0;
        col++;
    }

    cct_gotoxy(row, col);
    cout << "h(n):" << setw(4) << hfunc << "  g(n):" << setw(4) << gfunc << endl;
    col++;
    cct_gotoxy(row, col);
    int row_0 = row; //�����������
    int col_0 = col;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (!matrix[i][j]) //����ǿհ׸�
                cct_setcolor(COLOR_HWHITE, COLOR_HWHITE);
            else if (type == ep_node) //��չ�����ɽ��������ɫ��ͬ
                cct_setcolor(matrix[i][j], COLOR_HWHITE);
            else
                cct_setcolor(matrix[i][j], COLOR_BLACK);
            for (int k = 0; k < 3; k++)
            {
                cct_gotoxy(row, col + k);   //��λ����k��
                if (k == 1 && matrix[i][j]) //������м����ҷǿհ׸�
                    cout << "  " << matrix[i][j] << "   ";
                else
                    cout << pattern;
            }
            row += 6; //��λ����һ������
        }
        col_0 += 3;  //������������
        row = row_0; // row col�ص���������
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
}

void board::cal()
{
    hash_code = 0;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            hash_code = hash_code * 10 + matrix[i][j]; // ����״̬��ϣ��
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
    set<ll> s; // ����Ѿ��ڶ����л�����չ��ɵĽڵ�
    start_state.cal_hfunc(goal_state);
    start_state.gfunc = 0;
    q.push(start_state);
    s.insert(start_state.hash_code);
    cout << "��ʼ״̬Ϊ:";
    start_state.output(ep_node);
    while (!q.empty())
    {
        vector<board> t;
        board temp = q.top();
        f.push(temp);
        q.pop();
        step++; //��չ�����+1
        t.push_back(temp);
#if 0
        cout << endl
             << endl
             << endl;
        cout << "��Ҫ��չ";
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
            if (s.find(nxt_state.hash_code) == s.end()) // �ýڵ�û�з��ʹ�
            {
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

int cal_reverse(int array[width * height - 1]) //���������
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
    for (int i = 0, k = 0; i < width; i++) //����ά�������һά������
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
    return cnt1 % 2 == cnt2 % 2; //����Ը�����ͬ �����н�
}

void init(int mode)
{
    cct_cls(); //����
    cct_setcolor();
    cct_setconsoleborder(120, 120, 120, 9000); //���ÿ���̨(������)��С
    cct_setcursor(CURSOR_VISIBLE_NORMAL);
    if (mode == star_m)
        cct_setfontsize("������", 16, 8); //��������
    else if (mode == menu_m)
        cct_setfontsize("������", 24, 16); //��������
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
        cout << "��Ҫ��չ";
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
    for (int i = 0; i < width; i++) //���������
        for (int j = 0; j < height; j++)
            cin >> start[i][j];
    cout << menu_list[2] << endl;
    for (int i = 0; i < width; i++)
        for (int j = 0; j < height; j++)
            cin >> goal[i][j];
    int res = judge(start, goal); //�ж��Ƿ��н�
    if (!res)
        return -1;
    cout << menu_list[7] << endl;
    cin >> select_h;
    if (cin.fail() || select_h < 1 || select_h > 2)
    {
        cin.clear();
        select_h = m_dist; //ȱʡΪ�����پ���
    }
    cout << menu_list[3] << endl;
    cout << menu_list[4] << "   " << menu_list[5] << endl;
    cin >> select_op;
    if (cin.fail() || select_op <= 1 || select_op > 2)
    {
        cin.clear();
        select_op = 1; // Ĭ�ϻ�������
    }

    else
    {
        int time_op = 0;
        cout << menu_list[6] << endl;
        cin >> time_op; //����ʱ��ѡ��
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
        cout << "�޷��ӳ�ʼ״̬�ִ�����״̬" << endl;
        system("pause");
        return -1;
    }
    init(star_m);
    LARGE_INTEGER tick, begin, end;
    QueryPerformanceFrequency(&tick); //��ȡ������Ƶ��
    QueryPerformanceCounter(&begin);  //��ȡ��ʼ��¼
    A_star(board(start), board(goal));
    QueryPerformanceCounter(&end); //��ȡ��ֹ��¼
    // cout << endl
    //<< endl;
    // cout << "�ѴﵽĿ��״̬��" << endl;
    // cout << "��������Ч��Ϊ��" << endl;
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
    cout << "��չ�����Ϊ��" << step << endl;
    cout << "����ʱ��Ϊ��";
    cout << setiosflags(ios::fixed) << setprecision(6) << double(end.QuadPart - begin.QuadPart) / tick.QuadPart << "��" << endl;
    system("pause");
    return 0;
}
// 8 6 7 0 5 1 4 3 2