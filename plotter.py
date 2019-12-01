from matplotlib import pyplot as plt
# import matplotlib as plt

#pyplot.plot()


M = 2779191

def draw_occurences(endindex):
    filename = "./build/occurences.data"
    with open(filename, "r") as file:
        data = file.read().split("\n")[:-1][:endindex]
        occurences = list(map(lambda x: int(x), data))
        xaxis = list(range(len(occurences)))
        plt.plot(xaxis, occurences)
        plt.show()

def parseKDP(x):
    xx = x.split(",")
    return (int(xx[0]), int(xx[1]))

def draw_kdp(mfac):
    filename = "./build/k-d-pair.data"
    with open(filename, "r") as file:
        data = file.read().split("\n")[:-2]#[:endindex]
        kd = list(map(lambda x: parseKDP(x), data))
        kv = list(filter(lambda x: x[1] > 0, kd))
        kd = []
        for [k,v] in kv:
            if v < M * mfac: # Stop when we are > M * mfac, otherwise graph is unreadable
                kd.append([k,v])
            else:
                break
        x = list(map(lambda x: x[0], kd))
        y = list(map(lambda x: x[1], kd))
        plt.plot(x, y)
        mx = [x[0], x[len(x)-1]]
        my = [M, M]
        plt.plot(mx, my)
        plt.show()
draw_occurences(20)
draw_occurences(100)
draw_occurences(1000)
draw_kdp(2)
draw_kdp(4)

