#!/bin/bash
# ECLYRA - Prototipo iOS: genera il progetto Xcode e compila.
#
#   ./build_ios.sh                      compila per il Simulatore (non serve firmare)
#   ./build_ios.sh TEAMID               compila per iPhone/iPad collegato, firmato con il tuo Team ID
#   ./build_ios.sh TEAMID --open        genera e apre il progetto in Xcode (poi premi Run)
#
# Il Team ID si trova in Xcode > Settings > Accounts (va bene anche un Apple ID gratuito:
# l'app resta installata 7 giorni). Variabile opzionale: BUNDLE_ID (deve essere unico).
set -euo pipefail
cd "$(dirname "$0")"

TEAM_ID="${1:-${ECLYRA_TEAM_ID:-}}"
BUNDLE_ID="${BUNDLE_ID:-com.eclyra.proto.$(whoami | tr -cd 'a-z0-9')}"
APRI="${2:-}"

command -v cmake >/dev/null || { echo "Serve CMake 3.21+ (brew install cmake)"; exit 1; }
command -v xcodebuild >/dev/null || { echo "Serve Xcode (App Store) e: sudo xcode-select -s /Applications/Xcode.app"; exit 1; }

cmake -S . -B build-ios -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DECLYRA_TEAM_ID="$TEAM_ID" \
  -DECLYRA_BUNDLE_ID="$BUNDLE_ID"

if [ "$APRI" = "--open" ]; then
  open build-ios/ECLYRA_PROTO.xcodeproj
  echo "In Xcode: schema 'eclyra_proto', scegli il tuo iPhone in alto, poi Run."
  exit 0
fi

if [ -z "$TEAM_ID" ]; then
  echo "== Simulatore (nessuna firma)"
  xcodebuild -project build-ios/ECLYRA_PROTO.xcodeproj -scheme eclyra_proto \
    -configuration Debug -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' \
    CODE_SIGNING_ALLOWED=NO build | tail -20
  echo "App: $(find build-ios -name 'EclyraProto.app' -path '*iphonesimulator*' | head -1)"
  echo "Per provarla: xcrun simctl install booted <app> && xcrun simctl launch booted $BUNDLE_ID"
else
  echo "== Dispositivo (Team $TEAM_ID, bundle $BUNDLE_ID)"
  xcodebuild -project build-ios/ECLYRA_PROTO.xcodeproj -scheme eclyra_proto \
    -configuration Debug -destination 'generic/platform=iOS' -allowProvisioningUpdates build | tail -20
  echo "Installa e avvia da Xcode (./build_ios.sh $TEAM_ID --open) oppure con:"
  echo "  xcrun devicectl device install app --device <id> \$(find build-ios -name EclyraProto.app -path '*iphoneos*' | head -1)"
fi
