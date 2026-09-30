# Hisse Beyan Yardımcısı 🇹🇷

A desktop application to help Turkish citizens calculate income tax on foreign stock gains.

## Features

- Add and delete stock positions; search stock symbols via Yahoo Finance autocomplete
- Calculates tax base for closed positions and potential tax base for open positions, automatically fetching USD exchange rates and inflation indices from TCMB EVDS API
- Supports multi-position selection for batch close, delete, and potential-tax calculations
- Configurable tax rate (15%, 20%, 27%, 35%, 40%) and declaration limit
- Configurable EVDS API key stored persistently via the Settings menu
- Displays all positions in a sortable table (ID, symbol, name, quantity, buy/sell date & price, status, tax base)
- Summarizes total tax base and estimated tax liability
- Modern Qt6 GUI
- Comprehensive logging system with file and console output

## Logging

The application includes an integrated Qt-native logger that captures all application events. Logs are written to `tax_calc.log` in the application directory with timestamps, severity levels, and component categories.

**Default log level**: INFO (shows informational messages, warnings, and errors)

**Debug mode**: Run with `--debug` or `-d` flag to enable detailed debug logging:
```sh
./build/tax_calc --debug
```

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
# Run normally (INFO log level)
./build/tax_calc

# Run with debug logging enabled
./build/tax_calc --debug

# Show help and available options
./build/tax_calc --help
```

## EVDS API Key Setup

This application requires a free EVDS API key from TCMB to fetch exchange rates and inflation indices.

1. Go to [https://evds3.tcmb.gov.tr/login](https://evds3.tcmb.gov.tr/login) and register or log in.
2. Navigate to **Profilim** from the top-right dropdown and click **API KEY KOPYALA** at the bottom of the page.
3. In the application, open **Ayarlar → EVDS anahtarını değiştir...**, paste the key, and click **Tamam**.

For step-by-step instructions, use the **Yardım → EVDS anahtarı hakkında** menu inside the application.

## Usage

- Click **"Yeni Pozisyon Oluştur"** to open the create dialog. Start typing a ticker symbol to search Yahoo Finance and select from the autocomplete list.
- Select one or more rows in the table and click **"Seç"** to load them into the active selection.
- To close selected positions, enter the sell price and date, then click **"Pozisyonu Kapat"**. Batch operations run sequentially and show a progress dialog.
- Delete selected positions with **"Pozisyonu Sil"**.
- Calculate potential tax base for open positions with **"Hesapla"**; results accumulate across the selection.
- Adjust the **tax rate** and **declaration limit** to update the estimated tax liability instantly.
- Clear the selection at any time with **"Seçimi Temizle"**.

## Tax Calculation

The tax base for a closed position is calculated as:

```
sell_proceeds = ExchangeRate_sell × (SellPrice × Qty − 1.5)
buy_cost      = ExchangeRate_buy  × (BuyPrice  × Qty + 1.5)
inflationScaler = InflationIndex_sell / InflationIndex_buy

if inflationScaler >= 1.10:
    taxBase = sell_proceeds − buy_cost × inflationScaler
else:
    taxBase = sell_proceeds − buy_cost

taxBase = max(taxBase, 0)
```

The ±1.5 terms represent per-trade brokerage commission (fixed). Tax is applied only when the total tax base exceeds the declaration limit.

## Data Sources

- **TCMB EVDS API**: https://evds3.tcmb.gov.tr/  
  USD Exchange Rate: `TP.DK.USD.A` — Inflation Index: `TP.TUFE1YI.T1`
- **Yahoo Finance search API**: used for stock symbol lookup and autocomplete when creating a new position
