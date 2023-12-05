const axios = require('axios')

const API_URL = 'http://192.168.1.121/status' // Replace with your API URL

async function pingApi () {
  try {
    const response = await axios.get(API_URL)
    console.log('Response:', response.data)
  } catch (error) {
    console.error('Error:', error)
  }
}

setInterval(pingApi, 100) // 100ms interval for 10 times per second
