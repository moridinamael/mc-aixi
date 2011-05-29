import os
import re

import matplotlib.pyplot as plt
import numpy as np

extensions = [".png"] # Save graphs with these extensions

def split(s):
    return [x.strip() for x in s.split(",")]    

def parse(path):
    lines = open(path).readlines()
    labels = split(lines[0])
    lines = np.array([split(line) for line in lines[1:]])
    data = dict((label, lines[:, i]) for i, label in enumerate(labels))

    # Attempt conversion to numbers
    for label in labels:
        try:
            data[label] = data[label].astype(float)
        except:
            pass

    return data

def moving_average(data, size):
    if len(data) < size:
        return data.cumsum() / np.arange(1, len(data) + 1)
    cumsum = data.cumsum()
    ave = np.zeros_like(data)
    ave[size:] = (cumsum[size:] - cumsum[:-size]) / size
    ave[:size] = cumsum[:size] / np.arange(1, size + 1)
    return ave

def save_fig(path):
    for ext in extensions:
        plt.savefig(path + ext)

def graph_average_reward(data):
    cycle = data["cycle"]
    ave_reward = data["average reward"]

    fig = plt.figure()
    plt.title("Average reward vs. cycle")
    plt.xlabel("Cycle")
    plt.ylabel("Average reward")
    plt.plot(cycle, ave_reward)

    return fig

def graph_reward(data, window_size=1):
    cycle = data["cycle"]
    ave_reward = moving_average(data["reward"], window_size)

    fig = plt.figure()
    plt.title("Reward vs. cycle (window size = %d)" % window_size)
    plt.xlabel("Cycle")
    plt.ylabel("Reward")
    plt.plot(cycle, ave_reward)

    return fig

def graph_time_per_cycle(data, window_size=1):
    cycle = data["cycle"]
    time = moving_average(data["time"], window_size)
    
    fig = plt.figure()
    plt.title("Time per cycle vs. cycle (window size = %d)" % window_size)
    plt.xlabel("Cycle")
    plt.ylabel("Time per cycle [seconds]")
    plt.plot(cycle, time)

    return fig

def graph_model_size(data):
    cycle = data["cycle"]
    model_size = data["model size"]
    
    fig = plt.figure()
    plt.title("Model size vs. cycle")
    plt.xlabel("Cycle")
    plt.ylabel("Model size [number of nodes]")
    plt.plot(cycle, model_size)

    return fig



# Window size for moving average of reward
reward_window = 100

# Window size for moving average of time per cycle
time_window = 100

for log_file in os.listdir("log"):
    graph_dir = os.path.join("graph", log_file)

    if not os.path.isfile(os.path.join("log", log_file)):
        continue

    if not os.path.exists(graph_dir):
        os.makedirs(graph_dir)


    try:
        data = parse(os.path.join("log", log_file))
    
        graph_average_reward(data)
        save_fig(os.path.join(graph_dir, "average reward"))
    
        graph_reward(data, window_size=reward_window)
        save_fig(os.path.join(graph_dir,
                              "reward (window size = %d)" % reward_window))


        graph_time_per_cycle(data, window_size=time_window)
        save_fig(os.path.join(graph_dir,
                              "time per cycle (window size = %d)" % time_window))

        graph_model_size(data)
        save_fig(os.path.join(graph_dir, "model size"))

    except:
        pass

