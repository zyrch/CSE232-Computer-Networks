import os

response_time = {}

def get_dic(DIR):
    times = {}
    for file in os.listdir(f'{DIR}'):
        with open(f'{DIR}/{file}', 'r') as f:
            data = float(f.readline())
            print(data);
            clients = int(file[6:file.find('.')])
            times[clients] = data
    return times


import matplotlib.pyplot as plt

def plot(times):
    plt.scatter(times.keys(), times.values(), s = 5)
    plt.xlabel('num processes')
    plt.ylabel('avg. response time (s)')
    plt.title('num processes vs response time')
    plt.show()

if __name__ == "__main__":

    DIR = "response_times"
    times = get_dic(DIR)
    plot(times)
