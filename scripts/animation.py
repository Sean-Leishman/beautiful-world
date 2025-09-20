import os
import json
import math
import subprocess
import copy

NUM_SPHERES = 10
BACKGROUND_COLOR = [0.25, 0.25, 0.25]


def load_shapes(index, positions, frames):
    shapes = []
    for i in range(NUM_SPHERES):
        positions[i] = (positions[i] + 1) % len(frames)
        el = frames[positions[i]]
        sphere = {
            "type": "sphere",
            "center": [el[0], el[2], el[1]],
            "radius": 0.2,
            "material":
                {
                    "ks": 0.1,
                    "kd": 0.9,
                    "specularexponent": 20,
                    "diffusecolor": [0.8, 0.5, 0.5],
                    "specularcolor": [1.0, 0, 0],
                    "isreflective": False,
                    "reflectivity": 0.9,
                    "isrefractive": False,
                    "refractiveindex": 1.5
            }
        }

        if i % 2 == 0:
            sphere['material']['isreflective'] = True
        else:
            if i % 3 == 0:
                sphere['material']['isreflective'] = True

        shapes.append(sphere)

    floor = [
        {
            "type": "triangle",
            "v0": [-10, -2, 10],
            "v1": [10, -2, 10],
            "v2": [10, -2, -12],
            "material":
            {
                "ks": 0.1,
                "kd": 0.9,
                "specularexponent": 20,
                "diffusecolor": [0.5, 0.8, 0.5],
                "specularcolor": [1.0, 1.0, 1.0],
                "isreflective": False,
                "reflectivity": 1.0,
                "isrefractive": False,
                "refractiveindex": 1.0
            }
        },
        {
            "type": "triangle",
            "v0": [-10, -2, -12],
            "v1": [-10, -2, 10],
            "v2": [10, -2, -12],
            "material":
            {
                "ks": 0.1,
                "kd": 0.9,
                "specularexponent": 20,
                "diffusecolor": [0.5, 0.8, 0.5],
                "specularcolor": [1.0, 1.0, 1.0],
                "isreflective": False,
                "reflectivity": 1.0,
                "isrefractive": False,
                "refractiveindex": 1.0
            }
        }
    ]

    wall1 = [
        {
            "type": "triangle",
            "v0": [-10, -2, -12],
            "v1": [10, -2, -12],
            "v2": [10, 6, -12],
            "material":
            {
                "ks": 0.1,
                "kd": 0.9,
                "specularexponent": 20,
                "diffusecolor": [0.5, 0.8, 0.5],
                "specularcolor": [1.0, 1.0, 1.0],
                "isreflective": False,
                "reflectivity": 1.0,
                "isrefractive": False,
                "refractiveindex": 1.0
            }
        },
        {
            "type": "triangle",
            "v0": [-10, -2, -12],
            "v1": [-10, 6, -12],
            "v2": [10, 6, -12],
            "material":
            {
                "ks": 0.1,
                "kd": 0.9,
                "specularexponent": 20,
                "diffusecolor": [0.5, 0.8, 0.5],
                "specularcolor": [1.0, 1.0, 1.0],
                "isreflective": False,
                "reflectivity": 1.0,
                "isrefractive": False,
                "refractiveindex": 1.0
            }
        }
    ]

    wall2 = copy.deepcopy(wall1)
    wall3 = copy.deepcopy(wall2)
    wall4 = copy.deepcopy(wall3)

    wall2[0]['v0'] = [-10, -2, -12]
    wall2[0]['v1'] = [-10, -2, 10]
    wall2[0]['v2'] = [-10, 6, -12]

    wall2[1]['v0'] = [-10, -2, 10]
    wall2[1]['v1'] = [-10, 6, 10]
    wall2[1]['v2'] = [-10, 6, -12]

    wall3[0]['v0'] = [-10, 6, 10]
    wall3[0]['v1'] = [-10, -2, 10]
    wall3[0]['v2'] = [10, 6, 10]

    wall3[1]['v0'] = [-10, -2, 10]
    wall3[1]['v1'] = [10, 6, 10]
    wall3[1]['v2'] = [10, -2, 10]

    wall4[0]['v0'] = [10, 6, 10]
    wall4[0]['v1'] = [10, -2, 10]
    wall4[0]['v2'] = [10, 6, -12]

    wall4[1]['v0'] = [10, -2, 10]
    wall4[1]['v1'] = [10, 6, -12]
    wall4[1]['v2'] = [10, -2, -12]

    shapes.extend(floor)
    shapes.extend(wall1)
    shapes.extend(wall2)
    shapes.extend(wall3)
    shapes.extend(wall4)
    return shapes


def load_camera(index, positions, rotations):
    camera = {
        "type": "pinhole",
        "width": 800,
        "height": 600,
        "position": positions,
        "lookAt": rotations,
        "upVector": [0, 1, 0],
        "fov": 45.0,
        "exposure": 0.1
    }
    return camera


def load_light():
    positions = [
        light_positions[light_index[0]],
        light_positions[light_index[1]]
    ]

    lights = [
        {
            "type": "arealight",
            "normal": [0, 1, 0],
            "size": [2, 4, 0],
            "position": positions[0],
            "intensity": [10, 10, 10]
        },
        {
            "type": "arealight",
            "normal": [0, 1, 0],
            "size": [2, 4, 0],
            "position": positions[1],
            "intensity": [10, 10, 10]
        },
    ]
    return lights


def lemniscate_position(a, t):
    x = a * math.cos(t)
    y = a * math.sin(t) * math.cos(t)
    z = a * math.sin(t)

    return x, y, z


def falling_position(initial_height, gravity, frame):
    # Simple physics formula: s = ut + 1/2 at^2
    # Assuming initial vertical velocity (u) is 0
    return initial_height - 0.5 * gravity * (frame ** 2)


def generate_positions(num_frames, a, initial_height):
    positions = []
    for frame in range(num_frames):
        t = -math.pi + 2 * math.pi * frame / num_frames
        x, y, z = lemniscate_position(a, t)
        positions.append((x/5, z/5, y/5))
    return positions


def camera_position(radius, frame, num_frames):
    angle = 2 * math.pi * frame / num_frames
    x = radius * math.cos(angle)
    y = radius * math.sin(angle)

    pos = [x, 0.5, y]
    return pos, [0 - pos[0], 0 - pos[1], 0 - pos[2]]


# Parameters
num_frames = 100
a = 5  # Scale of the lemniscate
initial_height = 50  # Initial height of the object

# Generate positions
figure_8 = generate_positions(num_frames, a, initial_height)

# Initial Position: Index of figure_8 is current position of index of array
positions = [i for i in range(len(figure_8)) if i % NUM_SPHERES == 0]

camera_positions = [-1.0, 0.5, -1.5]
camera_rotations = [0 - i for i in [-1, 0.5, -1.5]]

light_positions = [[x * 0.05, 1, 1] for x in range(-50, 50)]
light_index = [0, 50]

for frame in range(num_frames):
    curr_frame = {}
    curr_frame['nbounces'] = 6
    curr_frame['nsamples'] = 500
    curr_frame['rendermode'] = 'pathtracer'
    position, look_at = camera_position(5, frame, 100)
    curr_frame['camera'] = load_camera(frame, position, look_at)

    curr_frame['scene'] = {}
    curr_frame['scene']['backgroundcolor'] = BACKGROUND_COLOR
    curr_frame['scene']['lightsources'] = load_light()

    curr_frame['scene']['shapes'] = load_shapes(frame, positions, figure_8)

    frame_str = "".join('0' for i in range(3 - len(str(frame)))) + str(frame)
    filename = f'./materials/Animation/Scene/frame_{frame_str}.json'
    with open(filename, "w") as f:
        json.dump(curr_frame, f)

    out = f'./materials/Animation/Image/frame_{frame_str}.ppm'
    result = subprocess.run(
        ["./cmake-build-release/main", filename[1:], out[1:]])

filename = f'./materials/Animation/video.mp4'
os.system(f'ffmpeg -framerate 10 -i "./materials/Animation/Image/frame_%03d.ppm" -c:v libx264 -crf 25 -vf "scale=500: 500, format=yuv420p" -movflags +faststart {filename}'
          )
