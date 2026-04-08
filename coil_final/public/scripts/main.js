async function sendRequest() {
    const textbox = document.getElementById('request-box');
    const apiUrl = '/telemetry_request/'; // Replace with your actual URL

    textbox.value = 'Loading...';

    try {
      const response = await fetch(apiUrl);
      
      // Ensure we're handling raw text, not JSON
      const data = await response.text();
      
      textbox.value = data;
    } catch (error) {
      textbox.value = 'Error: ' + error.message;
    }
  }
  
document.getElementById("connect-form").addEventListener("submit", async function(event) {
      event.preventDefault();

      const strParam = document.getElementById("str").value;
      const intParam = parseInt(document.getElementById("num").value, 10);

      if (!strParam || isNaN(intParam)) {
        alert("Please provide valid parameters.");
        return;
      }

      const url = `/connect/${encodeURIComponent(strParam)}/${intParam}`;

      try {
        const response = await fetch(url, {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            // You can add any data you need to send with the POST request here
            additionalData: "Some example data"
          })
        });

        if (response.ok) {
          const data = await response.json();
          alert("Connected successfully: " + JSON.stringify(data));
        } else {
          alert("Failed to connect. Status: " + response.status);
        }
      } catch (error) {
        alert("Error: " + error.message);
      }
    });