
import turtle

t = turtle.Turtle()
t.speed(0)  # 最快速度

# 绘制圆形
t.circle(50)

# 绘制螺旋
for i in range(10000):
    t.circle(i)
    t.right(5)

turtle.done()