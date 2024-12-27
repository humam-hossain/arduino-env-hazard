import { useEffect, useState } from "react"
import { LineChart, Line, Legend, CartesianGrid, Tooltip, XAxis, YAxis } from "recharts"

const App = () => {
  const [sensorData, setSensorData] = useState([])

  useEffect(() => {
    getSensorData()
  }
  , [])

  const getSensorData = async() =>{
    try {
      const response = await fetch("http://192.168.0.101:8000/api/get-data/")
      const data = await response.json()
      // console.log(data)
      setSensorData(data)
    }catch (error) {
      console.error(error)
    }
  }

  const headers = [
    // "t",
    "samples",
    "r_25um",
    "ugm3_25um",
    // "pcs_25um",
    "r_1um",
    "ugm3_1um",
    // "pcs_1um",
  ]

  // Color palette
  const colorPalette = [
    "#4a148c", "#1b5e20", "#d32f2f", "#1976d2", "#0288d1", 
    "#0288d1", "#8e24aa", "#7b1fa2", "#388e3c", "#c2185b"
  ];
  
  return (
    <>
      <h1>Sensor Data</h1>
      <LineChart width={1600} height={800} data={sensorData} margin={{ top: 5, right: 30, left: 20, bottom: 5 }}>
        <CartesianGrid strokeDasharray="3 3" />
        <XAxis dataKey="id" />
        <YAxis />
        <Tooltip />
        <Legend />

        {headers.map((header, index) => {
          return <Line type="monotone" dataKey={header} stroke={colorPalette[index]} key={index}/>
        })}

      </LineChart>
    </>
  );
};

export default App;