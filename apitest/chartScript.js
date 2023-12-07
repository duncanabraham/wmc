const motorAddress='192.168.1.121';
const dataLimit = 100;
const ctx = document.getElementById('speedChart').getContext('2d');
const speedChart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [], // Time or Data Points
     datasets: [
    {
      label: 'Actual Speed',
      data: [],
      borderColor: 'rgb(75, 192, 192)',
      tension: 0.1
    }, {
      label: 'Target Speed',
      data: [],
      borderColor: 'rgb(255, 99, 132)',
      tension: 0.1
    } 
    // {
    //   label: 'Position',
    //   data: [],
    //   borderColor: 'rgb(128, 128, 222)',
    //   tension: 0.1
    // }
    ]
  },
  options: {
    scales: {
      y: {
        beginAtZero: true
      }
    }
  }
});

async function fetchData() {
  try {
    const response = await fetch('http://localhost:3000/data');
    return await response.json();
  } catch (error) {
    console.error('Failed to fetch data:', error);
    return null; // or some default data structure
  }
}

function updateChart(data) {
  // First, shift the oldest data points if we have reached the data limit.
  if (speedChart.data.labels.length >= dataLimit) {
    speedChart.data.labels.shift();
    speedChart.data.datasets.forEach(dataset => {
      dataset.data.shift();
    });
  }

  // Then, push the new data.
  speedChart.data.labels.push(new Date().toLocaleTimeString());
  speedChart.data.datasets[0].data.push(data.actualSpeedRPM);
  speedChart.data.datasets[1].data.push(data.targetSpeedRPM);
  // speedChart.data.datasets[0].data.push(data.position);

  // Finally, update the chart.
  speedChart.update();
}


async function updateData() {
  const data = await fetchData();
  updateChart(data);
  setTimeout(updateData, 50); // Call updateData again after 1 second
}

updateData(); // Initial call to start the process

// PID data sender
document.getElementById('kp').addEventListener('input', function() {
  document.getElementById('kp-value').innerText = this.value;
  sendPIDValues();
});

document.getElementById('ki').addEventListener('input', function() {
  document.getElementById('ki-value').innerText = this.value;
  sendPIDValues();
});

document.getElementById('kd').addEventListener('input', function() {
  document.getElementById('kd-value').innerText = this.value;
  sendPIDValues();
});

// Initialize the span values on page load
document.getElementById('kp-value').innerText = document.getElementById('kp').value;
document.getElementById('ki-value').innerText = document.getElementById('ki').value;
document.getElementById('kd-value').innerText = document.getElementById('kd').value;


// Function to send PID values to the server
async function sendPIDValues() {
  const kp = document.getElementById('kp').value;
  const ki = document.getElementById('ki').value;
  const kd = document.getElementById('kd').value;

  try {
      const response = await fetch(`http://${motorAddress}/setpid?kp=${kp}&ki=${ki}&kd=${kd}`, {
          method: 'GET' // Or 'POST', if your server is set up to handle POST requests
      });
      if (!response.ok) {
          throw new Error('Network response was not ok');
      }
      console.log('PID values updated:', { kp, ki, kd });
  } catch (error) {
      console.error('Failed to send PID values:', error);
  }
}

async function fetchInitialPIDValues() {
  try {
    const response = await fetch(`http://${motorAddress}/status`);
    const data = await response.json();

    // Set initial slider values and display spans
    document.getElementById('kp').value = data.pid.kp;
    document.getElementById('kp-value').innerText = data.pid.kp.toFixed(2); // Assuming you want to show 2 decimal places
    document.getElementById('ki').value = data.pid.ki;
    document.getElementById('ki-value').innerText = data.pid.ki.toFixed(2);
    document.getElementById('kd').value = data.pid.kd;
    document.getElementById('kd-value').innerText = data.pid.kd.toFixed(3);
  } catch (error) {
    console.error('Failed to fetch initial PID values:', error);
  }
}

fetchInitialPIDValues();