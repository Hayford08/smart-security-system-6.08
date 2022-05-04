import sqlite3

gestures_db = '/var/jail/home/team26/server_src/gestures.db'

def request_handler(request):
    # with sqlite3.connect(gestures_db) as c:
    #     default_gestures = c.execute("""SELECT * FROM gesture_acceleration""").fetchall()
    #     return len(default_gestures[3][1]), len(default_gestures[2][1])
    #     return default_gestures
    if request["method"] == "POST":
        ## HERE do training of gestures
        if "training" in request["form"]:
            direction = request["form"]["direction"]
            acceleration_data = request["form"]["accel_sequence"]
            acceleration_data = acceleration_data[:-1]
            acceleration_data = acceleration_data.split(",")
            acceleration_data = acceleration_data[::-1]
            acceleration_data = ",".join(acceleration_data)
            with sqlite3.connect(gestures_db) as c:
                c.execute("""DELETE FROM gesture_acceleration WHERE direction=?""",(request["form"]["direction"],));
                c.execute("INSERT INTO gesture_acceleration(direction, sequence) VALUES(?,?)", (direction, acceleration_data))
            return acceleration_data
        ## HERE compute correlation
        if "check" in request["form"]:
            acceleration_data = request["form"]["accel_sequence"]
            acceleration_data = acceleration_data[:-1]
            acceleration_data = acceleration_data.split(",")
            float_accel_data = []
            for elt in acceleration_data:
                float_accel_data.append(float(elt))
            
            with sqlite3.connect(gestures_db) as c:
                default_sequence = c.execute("""SELECT sequence FROM gesture_acceleration WHERE direction = ? """, (request["form"]["direction"],)).fetchone()[0];
                default_sequence = default_sequence.split(",");
                float_def_sequence = [float(x) for x in default_sequence]
                return correlation(float_accel_data, float_def_sequence)
                # if correlation(float_accel_data, float_def_sequence) > 0.1:
                #     return request["form"]["direction"]

def offset_and_normalize(inp):
    mu = 1/len(inp)*sum(inp)
    return [(el-mu)/sum([(el-mu)**2 for el in inp])**(1/2) for el in inp]


def correlation(x,y):
    corr = 0
    x_0 = offset_and_normalize(x)
    y_0 = offset_and_normalize(y)
    for i in range(0, len(x)):
        corr+=x_0[i]*y_0[i]
    return corr
    