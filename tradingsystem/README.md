
# Trading System

## Overview
The `Trading System` project is a C++ application designed to simulate and manage various aspects of a trading system. The project leverages modern C++ features (C++20) and follows a modular design with distinct header files and source files for each service.

## Features
- **Algo Execution Service**: Handles algorithmic trading executions.
- **Algo Streaming Service**: Manages streaming data for algorithmic trading.
- **Execution Service**: Executes trades based on specified criteria.
- **GUI Service**: Provides a graphical user interface for the trading system.
- **Historical Data Service**: Manages historical market data.
- **Inquiry Service**: Handles client inquiries about trades and positions.
- **Market Data Service**: Processes live market data.
- **Position Service**: Tracks and updates positions in real-time.
- **Pricing Service**: Computes and delivers pricing information.
- **Risk Service**: Monitors and evaluates trading risks.
- **Trade Booking Service**: Books and logs executed trades.

## Project Structure
```
.
├── include/                # Header files for various services
│   ├── algoexecutionservice.hpp
│   ├── algostreamingservice.hpp
│   ├── executionservice.hpp
│   ├── guiservice.hpp
│   ├── historicaldataservice.hpp
│   ├── inquiryservice.hpp
│   ├── marketdataservice.hpp
│   ├── positionservice.hpp
│   ├── pricingservice.hpp
│   ├── products.hpp
│   ├── riskservice.hpp
│   ├── simulateddata.hpp
│   ├── soa.hpp
│   ├── streamingservice.hpp
│   ├── tradebookingservice.hpp
│   ├── utils.hpp
├── src/                    # Source files
│   ├── main.cpp            # Entry point for the application
├── CMakeLists.txt          # Build configuration file
```

## Prerequisites
- **C++ Compiler**: GCC 13 or later
- **CMake**: Version 3.10 or later

## Building the Project
Follow these steps to build the project:

1. Clone the repository:
   ```sh
   git clone <repository_url>
   cd tradingsystem
   ```

2. Create a build directory:
   ```sh
   mkdir build
   cd build
   ```

3. Configure the build using CMake:
   ```sh
   cmake ..
   ```

4. Compile the project:
   ```sh
   make
   ```

5. Run the executable:
   ```sh
   ./tradingsystem
   ```

## Customizing the Project
### Adding New Services
1. Create a new header file in the `include` directory.
2. Add the corresponding implementation in the `src` directory.
3. Ensure the new header file is included in the necessary parts of the application.

### Updating the Build System
- If you add new source files, make sure to update the `add_executable` command in `CMakeLists.txt` to include them.