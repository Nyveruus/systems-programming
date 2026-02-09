import sys
import requests
from bs4 import BeautifulSoup

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} URL")
        return 1
    url = sys.argv[1]

    response = requests.get(url)
    response.raise_for_status()

    soup = BeautifulSoup(response.text, "html.parser")

    for p in soup.find_all("p"):
        print(p.get_text(strip=True))
    return 0
main()
