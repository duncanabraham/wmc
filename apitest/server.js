const express = require('express')
const axios = require('axios')
const cors = require('cors')
const app = express()

const API_URL = 'http://192.168.1.121/status' // Your API URL

app.use(cors()) // Enable CORS for frontend access

const API_TIMEOUT = 5000 // 5 seconds timeout

async function fetchData () {
  try {
    const response = await axios.get(API_URL, { timeout: API_TIMEOUT })
    return response.data
  } catch (error) {
    if (error.code === 'ECONNABORTED') {
      console.error('Error: API request timed out')
    } else if (error.response) {
      // The server responded with a status code outside the 2xx range
      console.error('Error:', error.response.status, error.response.data)
    } else if (error.request) {
      // The request was made but no response was received
      console.error('Error: No response received from API')
    } else {
      // Something else happened in setting up the request
      console.error('Error:', error.message)
    }
    return null
  }
}

app.get('/data', async (req, res) => {
  const data = await fetchData()
  if (data) {
    res.json(data)
  } else {
    res.status(500).send('Error fetching data')
  }
})

const PORT = 3000
app.listen(PORT, () => console.log(`Server running on port ${PORT}`))
