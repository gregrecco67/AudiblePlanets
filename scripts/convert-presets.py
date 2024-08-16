import os
import xml.etree.ElementTree as ET
import glob

def process_xml_file(file_path):
    tree = ET.parse(file_path)
    root = tree.getroot()

    for elem in root.findall(".//*[@uid]"):
        uid = elem.get('uid')
        if uid and uid.startswith("osc") and "volume" in uid:
            gain = elem.get('val')
            if gain and float(gain) >= 0.0:
                db_value = 20 * log10(float(gain))
                elem.set('val', str(db_value))
        if uid and uid.startswith("env") and "sustain" in uid:
            sustain = elem.get('val')
            if sustain and float(sustain) >= 0.0:
                db_value = 20 * log10(float(sustain)/100.0)
                elem.set('val', str(db_value))
    tree.write(file_path)

def main(directory):
    preset_files = glob.glob(os.path.join(directory, "*.xml"))
    for preset_file in preset_files:
        process_xml_file(preset_file)

if __name__ == "__main__":
    import sys
    from math import log10

    if len(sys.argv) != 2:
        print("Usage: python script.py <preset-directory>")
        sys.exit(1)

    directory = sys.argv[1]
    main(directory)