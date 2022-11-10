1. 설정법
	우선 ssid와 password에 사용할 WiFi 의 ssid와 password 를 입력해줍니다.(단 WiFi는 2.4GHZ만 지원)

	serverName에 자신의 Mobius 서버의 주소를 입력해줍니다.

	Name에는 센서의 이름을 입력합니다. (Sensor1, 거실, 안방 등)

2. 내용
	해당 코드는 온습도 센서와 적외선 장애물 감지 센서를 이용하여 현재 위치의 온습도를 측정하며,
	적외선 장애물 감지 센서를 이용하여 센서에 가습기가 붙었는지를 확인합니다.
	이 때 온습도 정보는 serverName/DATA/current 에 저장됩니다.
	적외선 센서의 데이터 정보는 serverName/DATA/local 에 저장되며 감지가 될 경우 0의 값을 가지며 감지가 안될경우 1의 값을 가집니다.
	또한 사용자가 원하는 습도값은 serverName/DATA/target에 저장이 됩니다.

3. 함수 설명
	1. createAE()
		해당 함수는 센서를 Mobius 서버에 등록하기 위한 함수입니다.
		resourcename(rn) 의 값을 Name 으로 설정하여 Mobius 서버에 센서를 등록시킵니다.
		만약 등록이 된 센서라면 HttpResponseCode는 409가 반환되며, 등록에 성공하였다면 200이 반환됩니다.

	2. createCnt()
		해당 함수는 등록된 ae에 컨테이너를 생성합니다. 생성하는 컨테이너는 다음과 같습니다.
			1. serverName/Name/DATA
			2. serverName/Name/DATA/humidity
			3. serverName/Name/DATA/local : 센서의 앞에 장애물이 있으면 0의 값을 가짐 / 가습기의 위치를 확인할 때 사용함
			4. serverName/Name/DATA/humidity/current : 센서의 현재 습도 정보가 저장됨
			5. serverName/Name/DATA/humidity/target : 유저의 AE에서 설정하는 값 / 사용자가 원하는 습도 값을 저장함

	3. sendHumidity()
		온습도 센서로부터 습도의 값을 읽어와 서버에 전송하는 함수입니다.
		DHTsensor.readTemperature() 함수를 사용하여 실수형의 습도값을 Humidity 변수에 저장합니다.
		센서가 이상하여 Nan 값을 출력할 때가 많으므로 Nan 값이 아닐때만 서버에 전송하게끔 설정하였습니다.

	4. sendInfrared()
		적외선 장애물 감지 센서의 값을 서버로 전송하는 함수입니다.
		loop 문에서 이 함수를 호출하며 장애물이 감지되었다면 0을 서버에 전송하고 아니라면 1을 서버에 전송합니다.

	5. setup()
		시리얼 통신을 지원합니다.
		처음에 설정했던 ssid와 password를 이용하여 센서는 Wifi에 연결합니다.
		연결에 성공했다면 createAE() 함수와 createCnt() 함수를 이용하여 서버에 AE와 Cnt를 생성합니다.

	6. loop()
		Now 함수와 Next_Setted_Time 변수를 사용하여 10초에 한번만 서버에 습도값을 보냅니다.
		delay() 함수를 사용하지 않은이유는 delay동안 loop문을 진행하지 않아 적외선 장애물 감지 센서의 값이 변하더라도 서버에 전송을 하지 못하기 때문입니다.
		현재 적외선 장애물 감지 센서의 값을 infrared 변수에 넣고 이전의 값을 temp 변수에 넣습니다.
		따라서 적외선 장애물 감지 센서의 값이 바뀔때만 서버에 전송할 수 있도록 구현하였습니다.
		
