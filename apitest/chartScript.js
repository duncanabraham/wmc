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