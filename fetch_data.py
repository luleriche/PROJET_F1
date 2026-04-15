import fastf1
import numpy as np

def rotate_points(points, angle_deg):
    theta = np.radians(angle_deg)
    c, s = np.cos(theta), np.sin(theta)
    R = np.array(((c, -s), (s, c)))
    rot_points = points @ R.T
    return rot_points

def gps_to_xy(lat, lon, zero_lat, zero_lon):
    # circonference Terre = 40 008 km
    # on a donc 40 008 / 360 ≃ 111,133 km / deg
    m_per_lat = 111133 
    m_per_lon = 111133 * np.cos(np.radians(zero_lat))
    x = (lon - zero_lon) * m_per_lon
    y = (lat - zero_lat) * m_per_lat
    print(lat, zero_lat)
    return x, y

def kml_to_dict(file_path, zero_lat, zero_lon, zero_angle):
    file = open(file_path, 'r', encoding='utf-8')
    file_content = file.read()
    dict = {}
    polygon_strings = file_content.split('<Placemark>')[1:]

    for parts in polygon_strings:
        name = parts.split('<name>')[1].split("</name>")[0]
        dict[name] = list()
        coos_gps_string =  parts.split('<coordinates>')[1].split("</coordinates>")[0].strip()
        for str in coos_gps_string.split():
            lon,lat = str.split(',')[:2]
            x, y = gps_to_xy(float(lat), float(lon), zero_lat, zero_lon)
            liste = rotate_points([[x,y]], zero_angle+track_angle)
            dict[name].append(liste[0])
    print(f"{file_path} lu correctement.")
    return dict

def create_tel_file(lap, file_name):
    lap_time = lap['LapTime'].total_seconds()
    telemetry = lap.get_telemetry().reset_index(drop=True)
    times = telemetry["Time"].dt.total_seconds().to_list()
    distances = telemetry['Distance'].to_list()
    speeds = telemetry['Speed'].to_list()

    positions = np.vstack([telemetry['X']/10, telemetry['Y']/10]).T
    positions = rotate_points(positions, track_angle)
    x = positions[:, 0].tolist()
    y = positions[:, 1].tolist()

    f = open("data/"+file_name+".txt", "w", encoding="utf-8")

    telemetry_size = len(times)
    f.write(str(telemetry_size)+'\n')
    for i in range(telemetry_size):
        f.write(f"{times[i]}  {x[i]} {-y[i]} {distances[i]} {speeds[i]}\n")

session = fastf1.get_session(2026, 'Australian Grand Prix', 'Q')
session.load()
circuit_info = session.get_circuit_info()
track_angle = circuit_info.rotation

lapRUS = session.laps.pick_fastest()
lapHAM = session.laps.pick_drivers("HAM").pick_fastest()

create_tel_file(lapRUS, 'rus_tel')
create_tel_file(lapHAM, 'ham_tel')

kml_file = "data/australia.kml"
_lat, _lon, _angle = -37.84875839569118, 144.97017928051926, 1.4200000000000002
track_dictionary = kml_to_dict(kml_file, _lat, _lon, _angle)

f = open("data/track.txt", "w", encoding="utf-8")
for key in track_dictionary:
    liste_points = track_dictionary[key]
    nb_points = len(liste_points)
    f.write(key + ' ' +str(nb_points) + '\n')
    for i in range(nb_points):
        f.write(str(liste_points[i][0]) + ' ' +str(-liste_points[i][1]) + '\n')