const motorAddress = '192.168.1.121'
const dataLimit = 100
const ctx = document.getElementById('speedChart').getContext('2d')
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
})

async function fetchData () {
  try {
    const response = await fetch('http://localhost:3000/data')
    return await response.json()
  } catch (error) {
    console.error('Failed to fetch data:', error)
    return null // or some default data structure
  }
}

function updateChart (data) {
  // First, shift the oldest data points if we have reached the data limit.
  if (speedChart.data.labels.length >= dataLimit) {
    speedChart.data.labels.shift()
    speedChart.data.datasets.forEach(dataset => {
      dataset.data.shift()
    })
  }

  // Then, push the new data.
  speedChart.data.labels.push(new Date().toLocaleTimeString())
  speedChart.data.datasets[0].data.push(data.actualSpeedRPM)
  speedChart.data.datasets[1].data.push(data.targetSpeedRPM)
  // speedChart.data.datasets[0].data.push(data.position);

  // Finally, update the chart.
  speedChart.update()
}

async function updateData () {
  const data = await fetchData()
  updateChart(data)
  setTimeout(updateData, 50) // Call updateData again after 1 second
}

updateData() // Initial call to start the process

// PID data sender
document.getElementById('kp').addEventListener('input', function () {
  document.getElementById('kp-value').innerText = this.value
  sendPIDValues()
})

document.getElementById('ki').addEventListener('input', function () {
  document.getElementById('ki-value').innerText = this.value
  sendPIDValues()
})

document.getElementById('kd').addEventListener('input', function () {
  document.getElementById('kd-value').innerText = this.value
  sendPIDValues()
})

// document.getElementById('speed').addEventListener('input', function () {
//   document.getElementById('speed-value').innerText = this.value
//   sendSpeedValue(this.value)
// })

async function sendSpeedValue (speed) {
  try {
    const response = await fetch(`http://${motorAddress}/setspeed?speed=${speed}`, {
      method: 'GET'
    })
    if (!response.ok) {
      throw new Error('Network response error')
    }
    console.log('Speed updated:', speed)
  } catch (error) {
    console.error('Failed to send speed value:', error)
  }
}

// Initialize the span values on page load
document.getElementById('kp-value').innerText = document.getElementById('kp').value
document.getElementById('ki-value').innerText = document.getElementById('ki').value
document.getElementById('kd-value').innerText = document.getElementById('kd').value

// Function to send PID values to the server
async function sendPIDValues () {
  const kp = document.getElementById('kp').value
  const ki = document.getElementById('ki').value
  const kd = document.getElementById('kd').value

  try {
    const response = await fetch(`http://${motorAddress}/setpid?kp=${kp}&ki=${ki}&kd=${kd}`, {
      method: 'GET' // Or 'POST', if your server is set up to handle POST requests
    })
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    console.log('PID values updated:', { kp, ki, kd })
  } catch (error) {
    console.error('Failed to send PID values:', error)
  }
}

async function fetchInitialPIDValues () {
  let response
  try {
    response = await fetch(`http://${motorAddress}/status`)
  } catch (e) {
    response = { json: generateMockData() }
  }
  const data = await response.json()

  // Set initial slider values and display spans
  document.getElementById('kp').value = data.pid.kp
  document.getElementById('kp-value').innerText = data.pid.kp.toFixed(2) // Assuming you want to show 2 decimal places
  document.getElementById('ki').value = data.pid.ki
  document.getElementById('ki-value').innerText = data.pid.ki.toFixed(2)
  document.getElementById('kd').value = data.pid.kd
  document.getElementById('kd-value').innerText = data.pid.kd.toFixed(3)
  // Set maximum speed for the speed slider
  const maxSpeed = data.maxSpeed
  document.getElementById('speed').max = maxSpeed
  document.getElementById('speed-value').innerText = '0'

  // Update UI elements
  document.getElementById('firmware-version').innerText = data.firmwareVersion
  document.getElementById('serial-number').innerText = data.serialNumber
  document.getElementById('calibrated').innerText = data.calibrated ? 'Yes' : 'No'
  document.getElementById('direction').innerText = data.direction
  document.getElementById('position').innerText = data.position
  document.getElementById('temperature').innerText = data.temperature.toFixed(2)
  document.getElementById('humidity').innerText = data.humidity.toFixed(2)
}

async function updateMotorStatus () {
  try {
    const response = await fetch(`http://${motorAddress}/status`)
    const data = await response.json()

    // Update UI elements
    document.getElementById('firmware-version').innerText = data.firmwareVersion
    document.getElementById('serial-number').innerText = data.serialNumber
    document.getElementById('calibrated').innerText = data.calibrated ? 'Yes' : 'No'
    document.getElementById('direction').innerText = data.direction
    document.getElementById('position').innerText = data.position
    document.getElementById('temperature').innerText = data.temperature.toFixed(2)
    document.getElementById('humidity').innerText = data.humidity.toFixed(2)
  } catch (error) {
    console.error('Failed to fetch motor status:', error)
  }
}
document.addEventListener('DOMContentLoaded', (event) => {
  fetchInitialPIDValues()
  updateMotorStatus() // Update once immediately
  setInterval(updateMotorStatus, 5000) // Then set the interval for regular updates
})
