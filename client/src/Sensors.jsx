import { LineChart, Line, Legend, CartesianGrid, Tooltip, XAxis, YAxis, ResponsiveContainer } from "recharts"
import { format } from "date-fns"
import "./sensors.css"

// Color palette
const colorPalette = [
    "#1b5e20", // Dark Green
    "#d32f2f", // Dark Red
    "#1976d2", // Dark Blue
    "#f57c00", // Deep Orange
    "#6a1b9a", // Deep Purple
    "#c2185b", // Crimson
    "#0288d1", // Teal Blue
    "#388e3c", // Forest Green
    "#ff5722", // Burnt Orange
    "#455a64", // Slate Gray
];

const sensors = [
    "t",
    "temp",
    "humidity",
    "flame",
    "MQ5_LPG",
    "MQ7_CO",
    "MQ8_H2",
    "MQ135_AQ",
    "samples",
    "ugm3_1um",
    "ugm3_25um",
];
  
const Sensors = ({ data }) => {
    if (!data || data.length === 0) {
        return <p>No data available to display.</p>;
    }
    return (
        <>
            <h2>Last 24 Hour(s) Data</h2>
            <div className="sensor-charts">
                {sensors.map((sensor, index) => {
                    return (
                        <div key={index} className="sensor-chart">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart height="100%" data={data} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
                                    <CartesianGrid strokeDasharray="3 3" />
                                    <XAxis 
                                    dataKey="timestamp" 
                                    tickFormatter={(timestamp) => format(new Date(timestamp), "MMM d, HH:mm")} 
                                    angle={90}
                                    textAnchor="start"
                                    height={100}
                                    />
                                    <YAxis />
                                    <Tooltip />
                                    <Legend />

                                    <Line type="monotone" dataKey={sensor} stroke={colorPalette[index]} dot={false} isAnimationActive={false}/>

                                    {/* {headers.map((header, index) => {
                                    return <Line type="monotone" dataKey={header} stroke={colorPalette[index]} key={index}/>
                                    })} */}

                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    ) 
                })}
            </div>
        </>
    )
}

export default Sensors;