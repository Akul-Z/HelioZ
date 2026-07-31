print("gemma.py started")
from google import genai
from dotenv import load_dotenv
from pathlib import Path
import os

# -------------------------------
# Load API Key from .env
# -------------------------------
env_path = Path(__file__).parent / ".env"
load_dotenv(dotenv_path=env_path)

api_key = os.getenv("GEMINI_API_KEY")

if not api_key:
    raise Exception("GEMINI_API_KEY not found in .env")

# -------------------------------
# Create Gemini Client
# -------------------------------
client = genai.Client(api_key=api_key)


# -------------------------------
# Generate AI Emergency Report
# -------------------------------
def generate_report(latitude, longitude, impact):

    prompt = f"""
You are an emergency response AI.

A smart helmet has detected a possible road accident.

Impact Force: {impact} g
Latitude: {latitude}
Longitude: {longitude}

Write exactly 3 short sentences.

Rules:
- State that a possible road accident has been detected.
- Mention the impact force and GPS coordinates.
- Recommend immediate emergency medical assistance.
- Keep the response under 50 words.
- Use a professional tone.
- Do not use bullet points.
"""

    response = client.models.generate_content(
        model="gemini-flash-latest",
        contents=prompt
    )

    return response.text


# -------------------------------
# Test
# -------------------------------
if __name__ == "__main__":

    report = generate_report(
        latitude=15.364123,
        longitude=75.124567,
        impact=2.4
    )

    print("\n========== AI REPORT ==========\n")
    print(report)