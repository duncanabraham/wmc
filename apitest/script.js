const rpmChart = document.querySelector('.rpm-chart')
const temperatureGauge = document.querySelector('.temperature-gauge')
const humidityGauge = document.querySelector('.humidity-gauge')

// Fetch motor data
fetch('/motor-data')
  .then(response => response.json())
  .then(data => {
    console.log(data)

    // Update status section
    updateStatusSection(data)

    // Update control section
    updateControlSection(data)

    // Update data visualizations
    updateRPMChart(data.rpmData)
    updateTemperatureGauge(data.temperature)
    updateHumidityGauge(data.humidity)

    // Set PID values based on input sliders
    const kpSlider = document.getElementById('kpSlider')
    const kpValue = kpSlider.value
    const kiSlider = document.getElementById('kiSlider')
    const kiValue = kiSlider.value
    const kdSlider = document.getElementById('kdSlider')
    const kdValue = kdSlider.value

    // Send updated PID values to motor controller
    fetch('/set-pid-values', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ kp: kpValue, ki: kiValue, kd: kdValue })
    })
      .then(response => response.json())
      .then(data => {
        console.log('PID values updated')
      })

    // Set target speed based on input slider
    const targetSpeedSlider = document.getElementById('targetSpeedSlider')
    const targetSpeedValue = targetSpeedSlider.value

    // Send updated target speed to motor controller
    fetch('/set-target-speed', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ targetSpeed: targetSpeedValue })
    })
      .then(response => response.json())
      .then(data => {
        console.log('Target speed updated')
      })
  })

// Update status section
function updateStatusSection (data) {
  const firmwareVersion = document.getElementById('firmwareVersion')
  firmwareVersion.innerText = data.firmwareVersion

  const serialNumber = document.getElementById('serialNumber')
  serialNumber.innerText = data.serialNumber

  const calibratedStatus = document.getElementById('calibratedStatus')
  calibratedStatus.innerText = (data.calibrated === true ? 'Yes' : 'No')

  const pidValues = data.pidValues.map(value => Math.round(value))
  const kpValue = pidValues[0]
  const kiValue = pidValues[1]
  const kdValue = pidValues[2]

  document.getElementById('kpValue').textContent = kpValue.toString()
  document.getElementById('kiValue').textContent = kiValue.toString()
  document.getElementById('kdValue').textContent = kdValue.toString()

  const direction = data.direction
  const directionText = (direction === 'clockwise' ? 'Clockwise' : 'Counter-clockwise')
  const directionElement = document.getElementById('direction')
  directionElement.textContent = directionText

  const minSpeed = data.minSpeed
  const maxSpeed = data.maxSpeed

  document.getElementById('minSpeed').textContent = minSpeed.toString()
  document.getElementById('maxSpeed').textContent = maxSpeed.toString()

  const position = data.position
  const positionText = (position === null ? 'N/A' : position.toString())
  const positionElement = document.getElementById('position')
  positionElement.textContent = positionText

  const actualSpeed = data.actualSpeed
  const actualSpeedRPM = actualSpeed * 60
  const actualSpeedRPMText = (actualSpeedRPM === null ? 'N/A' : actualSpeedRPM.toFixed(2))
  const actualSpeedElement = document.getElementById('actualSpeed')
  actualSpeedElement.textContent = actualSpeedRPMText

  const targetSpeed = data.targetSpeed
  const targetSpeedText = (targetSpeed === null ? 'N/A' : targetSpeed.toFixed(2))
  const targetSpeedElement = document.getElementById('targetSpeed')
  targetSpeedElement.textContent = targetSpeedText

  const temperature = data.temperature
  const temperatureText = (temperature === null ? 'N/A' : temperature.toFixed(2)) + '°C'
  const temperatureElement = document.getElementById('temperature')
  temperatureElement.textContent = temperatureText
}

const intervalId = setInterval(() => {
  // Fetch motor data
  fetch('/motor-data')
    .then(response => response.json())
    .then(data => {
      console.log(data)

      // Update status section
      updateStatusSection(data)

      // Update control section
      updateControlSection(data)

      // Update data visualizations
      updateRPMChart(data.rpmData)
      updateTemperatureGauge(data.temperature)
      updateHumidityGauge(data.humidity)
    })
}, 1000) // Update data every second

// Update control section
function updateControlSection (data) {
  const kpSlider = document.getElementById('kpSlider')
  kpSlider.value = data.pidValues[0] // Set kp slider to current kp value

  const kiSlider = document.getElementById('kiSlider')
  kiSlider.value = data.pidValues[1] // Set ki slider to current ki value

  const kdSlider = document.getElementById('kdSlider')
  kdSlider.value = data.pidValues[2] // Set kd slider to current kd value

  const targetSpeedSlider = document.getElementById('targetSpeedSlider')
  targetSpeedSlider.value = data.targetSpeed // Set target speed slider to current target speed
}

// Update RPM chart
function updateRPMChart (rpmData) {
  // Clear existing chart data
  const rpmChart = document.querySelector('.rpm-chart')
  rpmChart.innerHTML = ''

  // Create new chart data
  const chartData = rpmData.map(({ time, rpm }) => {
    return {
      time,
      rpm
    }
  })

  // Plot data on chart
  const chart = new Chart(rpmChart, {
    type: 'line',
    data: {
      labels: chartData.map(data => data.time),
      datasets: [{
        label: 'Actual RPM',
        data: chartData.map(data => data.rpm),
        yAxisID: 'y-axis-1'
      }, {
        label: 'Target RPM',
        data: chartData.map(data => data.targetSpeed),
        yAxisID: 'y-axis-2'
      }]
    },
    options: {
      responsive: true,
      title: {
        display: true,
        text: 'Actual and Target RPM'
      },
      tooltips: {
        enabled: true,
        mode: 'x-axis',
        intersect: false,
        callbacks: {
          label: function (item) {
            return item.yLabel
          }
        }
      },
      scales: {
        yAxes: [{
          id: 'y-axis-1',
          type: 'linear',
          position: 'left',
          display: true,
          ticks: {
            beginAtZero: true
          }
        }, {
          id: 'y-axis-2',
          type: 'linear',
          position: 'right',
          display: true,
          ticks: {
            beginAtZero: true
          }
        }]
      }
    }
  })
}

// Update temperature gauge
function updateTemperatureGauge (temperature) {
  const temperatureGauge = document.querySelector('.temperature-gauge')
  const gaugeValue = temperature + '°C'

  // Update gauge value
  const gaugeValueElement = document.querySelector('.temperature-gauge .value')
  gaugeValueElement.textContent = gaugeValue

  // Update gauge color based on temperature range
  const temperatureRange = Math.floor(temperature / 10)
  const gaugeColor = temperatureRanges[temperatureRange]
  const gaugeElement = document.querySelector('.temperature-gauge .gauge')
  gaugeElement.style.backgroundColor = gaugeColor
}

function updateHumidityGauge (humidity) {
  const humidityGauge = document.querySelector('.humidity-gauge')
  const gaugeValue = humidity.toFixed(2) + '%'

  // Update gauge value
  const gaugeValueElement = document.querySelector('.humidity-gauge .value')
  gaugeValueElement.textContent = gaugeValue

  // Update gauge color based on humidity range
  const humidityRange = Math.floor(humidity / 10)
  const gaugeColor = humidityRanges[humidityRange]
  const gaugeElement = document.querySelector('.humidity-gauge .gauge')
  gaugeElement.style.backgroundColor = gaugeColor
}
