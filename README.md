# Tax Calculator 🇹🇷

A desktop application to help Turkish citizens calculate income tax on foreign stock gains.

## Features

- Add, edit, and delete stock transactions
- Automatically fetch USD exchange rates and inflation indices from TCMB EVDS API
- Calculates tax base for closed positions and potential tax base for open positions
- Displays all positions in a sortable table
- Summarizes yearly tax base
- Modern Qt6 GUI
- Comprehensive logging system with file and console output

## Logging

The application includes an integrated Qt-native logger that captures all application events. Logs are written to `tax_calc.log` in the application directory with timestamps, severity levels, and component categories.

See [LOGGER_USAGE.md](LOGGER_USAGE.md) for detailed logging documentation.

## Build & Run

### Prerequisites

- Qt 6.9+ (Widgets, Network, Sql modules)
- CMake 3.5+
- C++17 compiler

### Build

```sh
git clone https://github.com/yourusername/tax_calc.git
cd tax_calc
cmake -B build
cmake --build build
```

### Run

```sh
./build/tax_calc
```

## Usage

- Click "Yeni Pozisyon Oluştur" to add a new stock position.
- Select a position from the table to view or edit details.
- Close a position by entering sell price/date and clicking "Pozisyonu Kapat".
- Delete a position with "Pozisyonu Sil".
- Calculate potential tax base for today with "Bugün Kapatırsam?" section.

## Data Sources

- TCMB EVDS API: https://evds2.tcmb.gov.tr/  
USD Exchange Rate: TP.DK.USD.A, Inflation Index: TP.TUFE1YI.T1
