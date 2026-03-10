#include <Windows.h>
#include <iostream>

// 移动鼠标的相对位置
void MoveMouseRelative(int dx, int dy)
{
    POINT currentPos;
    if (GetCursorPos(&currentPos)) {
        SetCursorPos(currentPos.x + dx, currentPos.y + dy);
    }
}

int main()
{
    std::cout << "鼠标自动移动程序已启动，按 Ctrl+C 退出..." << std::endl;
    std::cout << "程序将每隔10秒小范围移动鼠标" << std::endl;
    
    // 移动方向和距离
    const int moveDistance = 10;  // 每次移动的像素距离
    bool moveRight = true;        // 移动方向标志
    
    while (true) {
        // 根据方向移动鼠标
        if (moveRight) {
            MoveMouseRelative(moveDistance, 0);  // 向右移动
        } else {
            MoveMouseRelative(-moveDistance, 0); // 向左移动
        }
        
        // 切换方向
        moveRight = !moveRight;
        
        // 等待10秒
        Sleep(10000);
    }
    
    return 0;
}