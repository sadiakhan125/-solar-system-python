

import turtle
import math

# ── Window Setup ─────────────────────────────
screen = turtle.Screen()
screen.title("Solar System Animation | Computer Graphics | ")
screen.bgcolor("black")
screen.setup(width=900, height=700)
screen.tracer(0)  # Fast animation

# ── Pause toggle ─────────────────────────────
paused = False
def toggle_pause():
    global paused
    paused = not paused

screen.listen()
screen.onkeypress(toggle_pause, "space")
screen.onkeypress(turtle.bye, "q")

# ── Helper: draw filled circle ───────────────
def draw_circle(t, x, y, radius, color, outline=None):
    t.penup()
    t.goto(x, y - radius)
    t.pendown()
    t.fillcolor(color)
    t.pencolor(outline if outline else color)
    t.pensize(1)
    t.begin_fill()
    t.circle(radius)
    t.end_fill()

# ── Helper: draw orbit ring ──────────────────
def draw_orbit(t, radius):
    t.penup()
    t.goto(0, -radius)
    t.pendown()
    t.pencolor("#333333")
    t.pensize(1)
    t.circle(radius)

# ── Turtle for drawing ───────────────────────
pen = turtle.Turtle()
pen.hideturtle()
pen.speed(0)

# ── Star field ───────────────────────────────
import random
random.seed(42)
stars = [(random.randint(-440, 440), random.randint(-340, 340)) for _ in range(180)]

def draw_stars():
    pen.penup()
    for sx, sy in stars:
        pen.goto(sx, sy)
        pen.pendown()
        pen.pencolor("white")
        pen.pensize(random.choice([1, 1, 1, 2]))
        pen.dot(random.choice([1, 1, 2]))
        pen.penup()

# ── Planet data ──────────────────────────────
# name, orbit_radius, size, color, speed, start_angle
planets = [
    {"name": "Mercury", "orbit": 70,  "size": 6,  "color": "#b5b5b5", "speed": 4.7,  "angle": 0},
    {"name": "Venus",   "orbit": 105, "size": 10, "color": "#e8cda0", "speed": 3.5,  "angle": 45},
    {"name": "Earth",   "orbit": 145, "size": 11, "color": "#4fa3e0", "speed": 2.9,  "angle": 120},
    {"name": "Mars",    "orbit": 185, "size": 8,  "color": "#c1440e", "speed": 2.4,  "angle": 200},
    {"name": "Jupiter", "orbit": 240, "size": 22, "color": "#c88b3a", "speed": 1.3,  "angle": 80},
    {"name": "Saturn",  "orbit": 295, "size": 18, "color": "#e4d191", "speed": 0.97, "angle": 160},
    {"name": "Uranus",  "orbit": 338, "size": 13, "color": "#7de8e8", "speed": 0.68, "angle": 300},
]

# Earth moon
moon = {"orbit": 22, "size": 4, "color": "#cccccc", "speed": 13.0, "angle": 0}

# ── Label turtle ─────────────────────────────
label = turtle.Turtle()
label.hideturtle()
label.speed(0)

# ── HUD turtle ───────────────────────────────
hud = turtle.Turtle()
hud.hideturtle()
hud.speed(0)

def draw_hud():
    hud.clear()
    hud.penup()
    hud.goto(-440, 320)
    hud.pencolor("#ffdd44")
    hud.write("Solar System Animation", font=("Arial", 13, "bold"))
    hud.goto(-440, 298)
    hud.pencolor("#888888")
    hud.write("Space: Pause/Resume   Q: Quit", font=("Arial", 10, "normal"))
    if paused:
        hud.goto(-30, 0)
        hud.pencolor("#ff4444")
        hud.write("⏸ PAUSED", font=("Arial", 16, "bold"))

# ── Draw static background (once) ────────────
pen.clear()
draw_stars()

# Draw orbit rings
for p in planets:
    draw_orbit(pen, p["orbit"])

# ── Main animation loop ───────────────────────
frame = 0
while True:
    try:
        if not paused:
            frame += 1

        pen.clear()

        # Stars + orbits (redraw each frame for clean look)
        draw_stars()
        for p in planets:
            draw_orbit(pen, p["orbit"])

        # Sun
        draw_circle(pen, 0, 0, 32, "#FDB813", "#FFD700")
        # Sun glow
        pen.penup()
        pen.goto(0, -36)
        pen.pendown()
        pen.pencolor("#ff9900")
        pen.pensize(8)
        pen.circle(36)

        label.clear()

        # Planets
        for p in planets:
            if not paused:
                p["angle"] += p["speed"] * 0.4

            rad = math.radians(p["angle"])
            px = p["orbit"] * math.cos(rad)
            py = p["orbit"] * math.sin(rad)

            draw_circle(pen, px, py, p["size"], p["color"])

            # Saturn rings
            if p["name"] == "Saturn":
                pen.penup()
                pen.goto(px - p["size"] - 12, py)
                pen.pendown()
                pen.pencolor("#c8b560")
                pen.pensize(3)
                pen.goto(px + p["size"] + 12, py)

                # Moon of Earth
            if p["name"] == "Earth":
                if not paused:
                    moon["angle"] += moon["speed"] * 0.4
                mrad = math.radians(moon["angle"])
                mx = px + moon["orbit"] * math.cos(mrad)
                my = py + moon["orbit"] * math.sin(mrad)
                draw_circle(pen, mx, my, moon["size"], moon["color"])

            # Planet label
            label.penup()
            label.goto(px, py + p["size"] + 5)
            label.pencolor("#ffffff")
            label.write(p["name"], align="center", font=("Arial", 7, "normal"))

        # Sun label
        label.penup()
        label.goto(0, 35)
        label.pencolor("#FDB813")
        label.write("Sun", align="center", font=("Arial", 9, "bold"))

        draw_hud()
        screen.update()

    except turtle.Terminator:
        break
