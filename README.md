# DHT11 Temperature and Humidity Sensor

An Arduino UNO microcontroller-based temperature and humidity sensor using the DHT11 sensor. The project incorporates components such as an LCD, a push button, an LED, an RGB module, and the Arduino UNO microcontroller. The sensor accurately measures temperature and humidity. The readings are displayed on the LCD. Additionally, the RGB module indicates the fairness of environmental conditions.

## Readings

Upon first use, the LCD displays the word "Ready," waiting for the button press. Once the button is pressed, the LCD displays "Standby" and then proceeds to display the temperature and humidity readings. The errors are displayed next to the measurements to inform the user about the limitations of the accuracy of the sensor's measurements.

![Ready](/project_images/ready.png)<br>
Caption: Waiting for a button press.

![Standby](/project_images/standby.png)<br>
*Caption: Standby for readings.*

![Display of Readings](/project_images/readings.png)<br>
*Caption: Display of the readings and conditions.*

## Error Handling

If the DHT11 is disconnected or if the checksum is incorrect, the error LED is illuminated, and the RGB LED is turned off to indicate inconclusive conditions. This ensures that the user understands that an error has occurred. If the DHT11 sensor is connected again and the checksum is correct, the error LED turns off, and the program continues its normal operation.

![Error Indication](/project_images/error.png)<br>
*Caption: Error Handling.*
