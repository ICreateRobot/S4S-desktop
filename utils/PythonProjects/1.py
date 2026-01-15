import turtle
import random
import colorsys

# 设置窗口
screen = turtle.Screen()
screen.title("多彩螺旋动画 - Python Turtle")
screen.bgcolor("black")
screen.setup(width=800, height=600)

# 创建海龟对象
t = turtle.Turtle()
t.speed(0)  # 最快速度
t.width(2)

# 隐藏海龟
t.hideturtle()

# 提升绘图性能
screen.tracer(0)

def draw_colorful_spiral():
    """绘制多彩螺旋图案"""
    t.clear()  # 清空画布
    
    # 移动到起始位置
    t.penup()
    t.goto(0, 0)
    t.pendown()
    
    # 绘制螺旋
    for i in range(200):
        # 使用HSV颜色空间生成鲜艳的颜色
        hue = (i / 200)  # 0到1之间的色调
        saturation = 1.0
        value = 1.0
        
        # HSV转RGB
        rgb = colorsys.hsv_to_rgb(hue, saturation, value)
        color = (rgb[0], rgb[1], rgb[2])
        
        t.pencolor(color)
        
        # 绘制
        t.forward(i * 0.5)
        t.right(91)
        
    screen.update()

def draw_flower():
    """绘制彩色花朵"""
    t.clear()
    t.width(3)
    
    for _ in range(36):  # 36个花瓣
        # 随机颜色
        r = random.random()
        g = random.random()
        b = random.random()
        t.pencolor(r, g, b)
        
        # 绘制花瓣
        for _ in range(2):
            t.circle(100, 60)
            t.left(120)
        t.left(10)
    
    # 绘制花蕊
    t.penup()
    t.goto(0, 0)
    t.pendown()
    t.pencolor("yellow")
    t.width(5)
    t.fillcolor("gold")
    t.begin_fill()
    t.circle(20)
    t.end_fill()
    
    screen.update()

def draw_star():
    """绘制星星"""
    t.clear()
    t.width(2)
    
    for _ in range(50):
        x = random.randint(-350, 350)
        y = random.randint(-250, 250)
        size = random.randint(5, 20)
        
        t.penup()
        t.goto(x, y)
        t.pendown()
        
        # 随机颜色
        t.pencolor(random.random(), random.random(), random.random())
        
        # 绘制五角星
        t.begin_fill()
        for _ in range(5):
            t.forward(size)
            t.right(144)
        t.end_fill()
    
    screen.update()

def draw_mandala():
    """绘制曼陀罗图案"""
    t.clear()
    t.width(1)
    
    for i in range(72):
        # 计算颜色
        hue = i / 72
        rgb = colorsys.hsv_to_rgb(hue, 0.8, 1.0)
        t.pencolor(rgb[0], rgb[1], rgb[2])
        
        # 绘制圆形图案
        t.circle(100)
        t.circle(50)
        
        # 绘制线条
        for j in range(6):
            t.forward(100)
            t.backward(100)
            t.right(60)
        
        t.right(5)
    
    screen.update()

def draw_fractal_tree(branch_len, t):
    """递归绘制分形树"""
    if branch_len > 5:
        # 随着树枝变短，颜色从棕色渐变到绿色
        if branch_len < 20:
            t.pencolor("green")
        else:
            t.pencolor("brown")
        
        t.width(branch_len / 10)
        
        t.forward(branch_len)
        t.right(20)
        draw_fractal_tree(branch_len - 15, t)
        t.left(40)
        draw_fractal_tree(branch_len - 15, t)
        t.right(20)
        t.backward(branch_len)

def draw_tree():
    """绘制分形树"""
    t.clear()
    t.penup()
    t.goto(0, -200)
    t.setheading(90)  # 指向正上方
    t.pendown()
    t.speed(0)
    draw_fractal_tree(75, t)
    screen.update()

# 定义图案列表
patterns = [
    ("1. 多彩螺旋", draw_colorful_spiral),
    ("2. 彩色花朵", draw_flower),
    ("3. 星空", draw_star),
    ("4. 曼陀罗", draw_mandala),
    ("5. 分形树", draw_tree)
]

# 显示菜单
def show_menu():
    print("\n" + "="*50)
    print("Python Turtle 绘图程序")
    print("="*50)
    for pattern in patterns:
        print(pattern[0])
    print("6. 退出程序")
    print("="*50)

# 键盘控制
def on_key_press(key):
    """键盘按键处理"""
    try:
        key_num = int(key)
        if 1 <= key_num <= 5:
            patterns[key_num-1][1]()  # 调用对应的绘图函数
        elif key_num == 6:
            screen.bye()  # 关闭窗口
    except ValueError:
        pass

# 设置键盘监听
screen.listen()
for i in range(1, 7):
    screen.onkey(lambda num=i: on_key_press(str(num)), str(i))

# 初始绘制
draw_colorful_spiral()
show_menu()

# 鼠标点击切换图案
pattern_index = 0
def on_click(x, y):
    """鼠标点击切换图案"""
    global pattern_index
    pattern_index = (pattern_index + 1) % len(patterns)
    patterns[pattern_index][1]()
    print(f"\n已切换到: {patterns[pattern_index][0]}")

screen.onclick(on_click)

# 保持窗口打开
print("\n操作说明：")
print("- 按数字键1-6选择图案")
print("- 鼠标点击切换图案")
print("- 按6退出程序")

turtle.mainloop()
