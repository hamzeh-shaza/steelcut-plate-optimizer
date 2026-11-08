#include <iostream>
using namespace std;

int main() {
    cout << "========================================================\n";
    cout << "         WELCOME TO THE             \n";
    cout << "      SMARTCUT PLATE OPTIMIZER SYSTEM        \n";
    cout << "========================================================\n\n";

    int plateIds[100];
    double plateWidths[100];
    double plateLengths[100];
    int plateQuantities[100];
    int plateCount = 0;

    double pieceWidths[100];
    double pieceLengths[100];
    int pieceQuantities[100];
    int pieceRotations[100];
    int pieceCount = 0;
    double bladeKerf = 0.0;

    cout << "length and width in m, thickness is in mm" << endl;
    cout << "Enter the cutting blade thickness (kerf): ";
    cin >> bladeKerf;

    // Validate: kerf cannot be negative
    while (bladeKerf < 0) {
        cout << "Error: Blade kerf cannot be negative!\n";
        cout << "Enter the cutting blade thickness (kerf) again: ";
        cin >> bladeKerf;
    }

    cout << "Enter the total number of unique stock plate types available (Maximum 100): ";
    cin >> plateCount;
    if (plateCount <= 0 || plateCount > 100) {
        cout << "Error: Plate count must be between 1 and 100!\n";
        return 1;
    }

    // Input loop for stock plates
    for (int i = 0; i < plateCount; i++) {
        cout << "\n--- Stock Plate Type " << (i + 1) << " ---\n";
        cout << "Enter numeric ID for this plate type: ";
        cin >> plateIds[i];
        cout << "Enter width of this plate: ";
        cin >> plateWidths[i];
        cout << "Enter length of this plate: ";
        cin >> plateLengths[i];
        cout << "Enter available stock quantity for this plate: ";
        cin >> plateQuantities[i];

        if (plateWidths[i] <= 0 || plateLengths[i] <= 0 || plateQuantities[i] <= 0) {
            cout << "Error: Width, length, and quantity must be positive!\n";
            return 1;
        }
    }

    cout << "\nEnter the total number of unique required piece sizes needed (Maximum 100): ";
    cin >> pieceCount;
    if (pieceCount <= 0 || pieceCount > 100) {
        cout << "Error: Piece count must be between 1 and 100!\n";
        return 1;
    }

    // Input loop for required pieces
    for (int i = 0; i < pieceCount; i++) {
        cout << "\n--- Required Piece Type " << (i + 1) << " ---\n";
        cout << "Enter required width: ";
        cin >> pieceWidths[i];
        cout << "Enter required length: ";
        cin >> pieceLengths[i];
        cout << "Enter required quantity: ";
        cin >> pieceQuantities[i];
        cout << "Is rotation allowed for this piece? (Enter 1 for YES, 0 for NO): ";
        cin >> pieceRotations[i];

        if (pieceWidths[i] <= 0 || pieceLengths[i] <= 0 || pieceQuantities[i] <= 0) {
            cout << "Error: Width, length, and quantity must be positive!\n";
            return 1;
        }
        if (pieceRotations[i] != 0 && pieceRotations[i] != 1) {
            cout << "Error: Rotation must be 0 or 1!\n";
            return 1;
        }
    }

    // --- Optimization Section ---
    int bestPlateIndex = -1;
    int bestPlatesNeeded = 999999;
    double minWaste = 1e30;
    double maxEfficiency = -1.0;
    int totalPiecesCut = 0;

    for (int i = 0; i < pieceCount; i++)
        totalPiecesCut += pieceQuantities[i];

    for (int i = 0; i < plateCount; i++) {
        double plateWidth = plateWidths[i];
        double plateLength = plateLengths[i];
        double plateArea = plateWidth * plateLength;
        bool physicallyFits = true;

        // Check if all piece types physically fit on this plate
        for (int j = 0; j < pieceCount; j++) {
            double reqW = pieceWidths[j];
            double reqL = pieceLengths[j];

            // Normal orientation: kerf-adjusted fit formula
            int normalX =
                (plateWidth >= reqW) ? (int)((plateWidth + bladeKerf) / (reqW + bladeKerf)) : 0;
            int normalY = (plateLength >= reqL)
                              ?

                              (int)((plateLength + bladeKerf) / (reqL + bladeKerf))
                              : 0;
            int normalTotal = normalX * normalY;

            // Rotated orientation (if allowed)
            int rotTotal = 0;
            if (pieceRotations[j] == 1) {
                int rotX =
                    (plateWidth >= reqL) ? (int)((plateWidth + bladeKerf) / (reqL + bladeKerf)) : 0;
                int rotY = (plateLength >= reqW)
                               ? (int)((plateLength + bladeKerf) / (reqW + bladeKerf))
                               : 0;
                rotTotal = rotX * rotY;
            }

            int maxPerPlate = (rotTotal > normalTotal) ? rotTotal : normalTotal;
            if (maxPerPlate == 0) {
                physicallyFits = false;
                break;
            }
        }

        if (!physicallyFits)
            continue;

        // Compute total area needed
        double totalRequiredArea = 0;
        for (int j = 0; j < pieceCount; j++)
            totalRequiredArea += pieceWidths[j] * pieceLengths[j] * pieceQuantities[j];

        // Determine plates needed using area-based loop
        int platesNeeded = 0;
        double remaining = totalRequiredArea;
        while (remaining > 0 && platesNeeded < plateQuantities[i]) {
            platesNeeded++;
            remaining -= plateArea;
        }
        if (remaining > 0)
            continue; // Not enough stock

        double totalPlateAreaUsed = platesNeeded * plateArea;
        double currentWaste = totalPlateAreaUsed - totalRequiredArea;
        double currentEfficiency = (totalRequiredArea / totalPlateAreaUsed) * 100.0;

        // Selection: minimize waste, then maximize efficiency
        if (currentWaste < minWaste ||
            (currentWaste == minWaste && currentEfficiency > maxEfficiency) ||
            (currentWaste == minWaste && currentEfficiency == maxEfficiency &&
             platesNeeded < bestPlatesNeeded)) {
            minWaste = currentWaste;
            maxEfficiency = currentEfficiency;
            bestPlatesNeeded = platesNeeded;
            bestPlateIndex = i;
        }
    }

    // --- Output Section ---
    cout << "\n========================================================\n";
    cout << "             OPTIMIZATION RESULTS     \n";
    cout << "========================================================\n";

    if (bestPlateIndex != -1) {
        int remainingStock = plateQuantities[bestPlateIndex] - bestPlatesNeeded;

        cout << "Recommended Plate ID: " << plateIds[bestPlateIndex] << "\n";
        cout << "Recommended Plate Size: " << plateWidths[bestPlateIndex] << " x "
             << plateLengths[bestPlateIndex] << "\n";
        cout << "Number of Plates Needed: " << bestPlatesNeeded << "\n";
        cout << "Total Pieces Produced: " << totalPiecesCut << "\n";
        cout << "Total Waste Area: " << minWaste << "\n";
        cout << "Efficiency Percentage: " << maxEfficiency << "%\n";
        cout << "Remaining Stock Quantity of this Plate: " << remainingStock << "\n";
    } else {
        cout << "No suitable plate found in stock to fulfill the order constraints.\n";
    }
    cout << "========================================================\n";

    // --- Financial Invoice Section ---
    if (bestPlateIndex != -1) {
        double costPerSquareUnit = 0.0;
        double laborFeePerPlate = 0.0;
        double taxRatePercentage = 0.0;

        cout << "\n========================================================\n";
        cout << "       FINANCIAL COST & QUOTATION MODULE      \n";
        cout << "========================================================\n";

        cout << "Enter the material cost per square unit of area: ";
        cin >> costPerSquareUnit;
        cout << "Enter the cutting labor fee charged per plate: ";
        cin >> laborFeePerPlate;
        cout << "Enter the sales tax rate percentage (e.g., 15 for 15%): ";
        cin >> taxRatePercentage;

        double basePlateArea = plateWidths[bestPlateIndex] * plateLengths[bestPlateIndex];
        double totalMaterialCost = bestPlatesNeeded * basePlateArea * costPerSquareUnit;
        double totalLaborCost = bestPlatesNeeded * laborFeePerPlate;
        double subtotal = totalMaterialCost + totalLaborCost;
        double taxAmount = subtotal * (taxRatePercentage / 100.0);
        double finalTotalInvoice = subtotal + taxAmount;

        cout << "\n--- Financial Summary breakdown ---\n";
        cout << "Total Material Cost:           $" << totalMaterialCost << "\n";
        cout << "Total Labor/Cutting Cost: $" << totalLaborCost << "\n";
        cout << "--------------------------------------------------------\n";
        cout << "Subtotal:                 $" << subtotal << "\n";
        cout << "Tax Amount:                   $" << taxAmount << "\n";
        cout << "--------------------------------------------------------\n";
        cout << "FINAL CUSTOMER INVOICE: $" << finalTotalInvoice << "\n";
        cout << "========================================================\n";
    }

    return 0;
}
