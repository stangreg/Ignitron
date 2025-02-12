import json
import os
Import("env")

def build_preset_uuids(*args, **kwargs):
    inputDir = env.GetProjectOption("custom_data_dir", "data/")
    inputFile = "PresetList.txt"
    outputFile = "PresetListUUIDs.txt"

    fullPathInputFile = inputDir + inputFile
    fullPathOutputFile = inputDir + outputFile

    if not os.path.exists(fullPathInputFile):
        print(f"ERROR: File {fullPathInputFile} not found. Exiting.")
        assert(0)

    try:
        presetListFile = open(fullPathInputFile)
        outputFile = open(fullPathOutputFile, 'w')
    except Exception as exc:
        print("Exception while attempting to remove the folder '" + str(fullPathInputFile) + "': " + str(exc))

    for line in presetListFile:
        line = line.strip()
        if line.endswith(".json"):
            fullFileName = inputDir + line
            jsonFile = open(fullFileName, 'r')
            values = json.load(jsonFile)
            jsonFile.close()
            uuidValue = values['UUID']
            outputFile.write(line + ' ')
            outputFile.write(uuidValue + '\n')



env.AddCustomTarget(
    name="buildpresetuuids",
    dependencies=None,
    actions=[
        build_preset_uuids
    ],
    title="Build UUID file from Presets File",
    description="Extracts UUIDs from preset files for mapping"
)