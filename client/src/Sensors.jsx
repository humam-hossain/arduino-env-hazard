import { LineChart, Line, Legend, CartesianGrid, Tooltip, XAxis, YAxis, ResponsiveContainer } from "recharts"
import { format, parseISO, differenceInMinutes, differenceInHours } from "date-fns"
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

// Preprocess the data to introduce gaps if time difference > 2 minutes
const preprocessData = (data) => {
    if (!data || data.length === 0) return [];
    const processedData = [];
    const nullsPerHour = 50; // Number of null values to add per hour of gap

    for (let i = 0; i < data.length; i++) {
        processedData.push(data[i]); // Add the current data point

        if (i < data.length - 1) {
            const currentTime = parseISO(data[i].timestamp);
            const nextTime = parseISO(data[i + 1].timestamp);

            // Calculate the time gap in hours
            const timeGapHours = differenceInHours(nextTime, currentTime) + 
                                 differenceInMinutes(nextTime, currentTime) / 60;

            if (timeGapHours > 0) {
                // Calculate the number of null values to insert
                const nullValuesCount = Math.ceil(timeGapHours * nullsPerHour);

                for (let j = 0; j < nullValuesCount; j++) {
                    processedData.push({
                        ...data[i], // Preserve the structure of the data
                        timestamp: new Date(
                            currentTime.getTime() + ((j + 1) * (timeGapHours * 3600 * 1000)) / nullValuesCount
                        ).toISOString(), // Increment timestamp proportionally
                        ...Object.fromEntries(sensors.map((sensor) => [sensor, null])), // Set sensor values to null
                    });
                }
            }
        }
    }
    return processedData;
};


  
const Sensors = ({ data }) => {
    if (!data || data.length === 0) {
        return <p>No data available to display.</p>;
    }
    const processedData = preprocessData(data)
    // Get the latest sensor data
    const latestData = data[data.length - 1]

    return (
        <>
            <h2>Last 24 Hour(s) Data</h2>
            <div className="sensor-summary">
                <div className="sensor-grid">
                    {sensors.map((sensor, index) => (
                        <div
                            key={index}
                            className="sensor-item"
                            style={{ color: colorPalette[index % colorPalette.length] }} // Match the chart color
                        >
                            <span>{sensor}:</span>{" "}
                            <span>
                                {latestData[sensor] !== undefined && latestData[sensor] !== null
                                    ? latestData[sensor]
                                    : "N/A"}
                            </span>
                        </div>
                    ))}
                </div>
            </div>
            <div className="sensor-charts">
                {sensors.map((sensor, index) => {
                    return (
                        <div key={index} className="sensor-chart">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart height="100%" data={processedData} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
                                    <CartesianGrid strokeDasharray="3 3" />
                                    <XAxis
                                        dataKey="timestamp"
                                        tickFormatter={(timestamp) => format(new Date(timestamp), "MMM d, HH:mm")}
                                        angle={90}
                                        textAnchor="start"
                                        height={100}
                                    />
                                    <YAxis />
                                    <Tooltip
                                        formatter={(value, name) => [`${value}`, name]}
                                        labelFormatter={(label) => {
                                            const localTime = new Date(label).toLocaleString('en-US', {
                                            weekday: 'short',
                                            month: 'short',
                                            day: 'numeric',
                                            hour: '2-digit',
                                            minute: '2-digit',
                                            });
                                            return `Time: ${localTime}`;
                                        }}
                                    />
                                    <Legend />
                                    <Line
                                        type="monotone"
                                        dataKey={sensor}
                                        stroke={colorPalette[index]}
                                        dot={false}
                                        isAnimationActive={false}
                                    />
                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    );
                })}
            </div>
        </>
    )
}

export default Sensors;