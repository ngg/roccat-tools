# Soundfile conversions:
# oggenc --resample 44100 -q5 *.wav
# mmv "*.ogg" "#1.oga"

INSTALL(DIRECTORY cpi live_recording profile sensitivity timer
  DESTINATION share/roccat/sounds
)
