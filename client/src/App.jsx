import { useEffect, useState } from "react"
import { BrowserRouter as Router, Route, Routes, NavLink, useLocation } from "react-router-dom"
import Overview from "./Overview.jsx";
import Sensors from "./Sensors.jsx";
import "./App.css"

const App = () => {
  return (
    <Router>
      <Dashboard />
    </Router>
  );
};

const Dashboard = () => {
  const [overviewSensorData, setOverviewSensorData] = useState([]);
  const [sensorsData, setSensorsData] = useState([]);
  const [currentTime, setCurrentTime] = useState(new Date().toLocaleTimeString());
  const location = useLocation();

  // const SERVER_IP = "192.168.0.104:8000"
  const SERVER_IP = "192.168.0.104:8000"

  const duration = 10000
  useEffect(() => {
    // Update the current time every second
    const timer = setInterval(() => {
      setCurrentTime(new Date().toLocaleTimeString());
    }, duration);

    // Cleanup interval on component unmount
    return () => clearInterval(timer);
  }, []);

  useEffect(() => {
    // Start fetching data periodically
    if (location.pathname === "/") {
      const interval = setInterval(() => {
        getOverviewSensorData();
      }, duration); // Fetch every 5 seconds

      // Cleanup interval on component unmount
      return () => clearInterval(interval);
    }
    
    if(location.pathname === "/sensors"){
      const interval = setInterval(() => {
        getSensorsData();
      }, duration); // Fetch every 5 seconds

      return () => clearInterval(interval);
    }
  }, [location.pathname]);

  const timelapse = 24 * 60 * 60 * 1000
  const getOverviewSensorData = async () => {
    try {
      // Calculate the timestamp for 1 hour ago
      const from = new Date(Date.now() - timelapse).toISOString();

      const response = await fetch(`http://${SERVER_IP}/api/get-data/?from=${from}`);
      const data = await response.json();
      setOverviewSensorData(data);
    } catch (error) {
      console.error("[ERROR] fetching sensor data:", error);
    }
  };

  const getSensorsData = async () => {
    try {
      // Calculate the timestamp for 1 hour ago
      const from = new Date(Date.now() - timelapse).toISOString();

      const response = await fetch(`http://${SERVER_IP}/api/get-data/?from=${from}`);
      const data = await response.json();
      setSensorsData(data);
    } catch (error) {
      console.error("[ERROR] fetching sensor data:", error);
    }
  };

  return (
    <div className="dashboard">
      <header className="dashboard-header">
        Industrial Environment Quality Monitoring & Hazard Detection System {`Current Time: ${currentTime}`}
      </header>
      <div className="dashboard-body">
        <nav className="sidebar">
          <ul>
            <li>
              <NavLink to="/" className={({ isActive }) => (isActive ? "active" : "")}>
                Overview
              </NavLink>
            </li>
            <li>
              <NavLink to="/sensors" className={({ isActive }) => (isActive ? "active" : "")}>
                Sensors
              </NavLink>
            </li>
            <li>
              <NavLink to="/analysis" className={({ isActive }) => (isActive ? "active" : "")}>
                Analysis
              </NavLink>
            </li>
            <li>
              <NavLink to="/settings" className={({ isActive }) => (isActive ? "active" : "")}>
                Settings
              </NavLink>
            </li>
            <li>
              <NavLink to="/logs" className={({ isActive }) => (isActive ? "active" : "")}>
                Logs
              </NavLink>
            </li>
            <li>
              <NavLink to="/about" className={({ isActive }) => (isActive ? "active" : "")}>
                About
              </NavLink>
            </li>
          </ul>
        </nav>
        <main className="content">
          <div className="container">
            <Routes>
              <Route path="/" element={<Overview data={overviewSensorData} />} />
              <Route path="/sensors" element={<Sensors data={sensorsData} />} />
              <Route path="/analysis" element={<h1>Analysis</h1>} />
              <Route path="/settings" element={<h1>Settings</h1>} />
              <Route path="/logs" element={<h1>Logs</h1>} />
              <Route path="/about" element={<h1>About</h1>} />
            </Routes>
          </div>
        </main>
      </div>
      <footer className="dashboard-footer">
        <h3>Project Members:</h3> 
        <p>
          <a href="mailto:humam.hossain.e@gmail.com">Muhammed Humam Hossain</a>, <a href="mailto:mahmudurmarfy@gmail.com">Mahmudur Rahman Murphy</a>, <a href="mailto:202111075@ye.butex.edu.bd">Saihan Bin Sajjad</a>, <a href="mailto:202111020@ye.butex.edu.bd">Md Mahmudur Rahman</a>
        </p>
        <p>Yarn Engineering Department, Batch 47</p>
        <h3><a href="https://www.butex.edu.bd/homepage/">Bangladesh University of Textiles, Dhaka, Bangladesh</a></h3>
      </footer>

    </div>
  );
};

export default App;