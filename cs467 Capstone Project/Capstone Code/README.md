**Install the Google App Engine Python SDK:**
```
gcloud components install app-engine-python
gcloud components install app-engine-python-extras
````

**Install the Flask dependencies:**
```
python -m pip install -t lib -r requirements.txt
```

**Install the React dependencies:**
```
cd frontend
npm install
```

**Start the Flask server:**
```
python main.py
```

**Start the Webpack Dev Server:**
```
cd frontend
npm run start
```

**Go to this address to see the latest version of the page:**
http://localhost:8081

**Create the production build (do this before deploying):**
```
cd frontend
npm run build
```

**Deploy to Google Cloud:**
```
gcloud app deploy
```

**The files under the LSTM folder are hosted on our Nginx server separately.**

**The Python Scripts folder contains files we used in some way to produce either the appropriate file formats or show proof of concept for certain statistics.**
 
