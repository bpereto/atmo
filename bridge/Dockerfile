FROM python:3.8-slim

WORKDIR /app
COPY requirements.txt ./

RUN pip install -r requirements.txt

COPY bridge.py /app

CMD [ "python3", "/app/bridge.py" ]
