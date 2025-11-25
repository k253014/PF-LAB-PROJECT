#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

#define PROFILE_FILE "cfit_profile.dat"
#define LOG_FILE "cfit_log.dat"

typedef struct {
    char name[50];
    int age;
    char gender[10];
    float height_cm;
    float weight_kg;
    float target_weight_kg;
    int daily_calorie_goal;
    int is_profile_set;
} UserProfile;

typedef struct {
    char date[11];
    char meal_name[50];
    int calories;
    float protein_g;
    float carbs_g;
    float fat_g;
} MealLog;

UserProfile profile;

void get_current_date(char *date_str) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(date_str, 11, "%Y-%m-%d", tm);
}

float cm_to_meters(float cm) {
    return cm / 100.0;
}

void press_enter_to_continue() {
    printf("\nPress Enter to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
}

void load_profile() {
    FILE *file = fopen(PROFILE_FILE, "rb");
    if (file == NULL || fread(&profile, sizeof(UserProfile), 1, file) != 1) {
        printf("No existing profile found. You must create one.\n");
        profile.is_profile_set = 0;
        memset(profile.name, 0, sizeof(profile.name));
    } else {
        printf("Welcome back, %s!\n", profile.name);
        profile.is_profile_set = 1;
    }
    if (file) fclose(file);
}

void save_profile() {
    FILE *file = fopen(PROFILE_FILE, "wb");
    if (file == NULL) {
        printf("Error: Could not save profile file.\n");
        return;
    }
    fwrite(&profile, sizeof(UserProfile), 1, file);
    fclose(file);
    printf("\nProfile saved successfully!\n");
}

void create_or_update_profile(int is_update) {
    printf("\n--- %s PROFILE ---\n", is_update ? "UPDATE" : "CREATE");
    printf("Enter Name: ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (fgets(profile.name, sizeof(profile.name), stdin) != NULL)
        profile.name[strcspn(profile.name, "\n")] = 0;

    printf("Enter Age (years): ");
    scanf("%d", &profile.age);

    do {
        printf("Enter Gender (Male/Female): ");
        scanf("%s", profile.gender);
        char temp_gender[10];
        strcpy(temp_gender, profile.gender);
        for (int i = 0; temp_gender[i]; i++) temp_gender[i] = tolower(temp_gender[i]);
        if (strcmp(temp_gender, "male") != 0 && strcmp(temp_gender, "female") != 0)
            printf("Invalid gender. Please enter 'Male' or 'Female'.\n");
        else {
            profile.gender[0] = toupper(profile.gender[0]);
            break;
        }
    } while (1);

    printf("Enter Height (cm): ");
    scanf("%f", &profile.height_cm);
    printf("Enter Weight (kg): ");
    scanf("%f", &profile.weight_kg);

    printf("\n--- GOAL SETTING ---\n");
    printf("Enter Target Weight (kg): ");
    scanf("%f", &profile.target_weight_kg);
    printf("Enter Daily Calorie Goal (kcal): ");
    scanf("%d", &profile.daily_calorie_goal);

    profile.is_profile_set = 1;
    save_profile();
}

void log_meal() {
    if (!profile.is_profile_set) {
        printf("\nError: Please create your profile first.\n");
        press_enter_to_continue();
        return;
    }

    MealLog new_log;
    get_current_date(new_log.date);

    printf("\n--- LOG NEW MEAL ENTRY for %s ---\n", new_log.date);
    printf("Enter Meal Name: ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    if (fgets(new_log.meal_name, sizeof(new_log.meal_name), stdin) != NULL)
        new_log.meal_name[strcspn(new_log.meal_name, "\n")] = 0;

    printf("Enter Total Calories (kcal): ");
    scanf("%d", &new_log.calories);
    printf("Enter Protein (g): ");
    scanf("%f", &new_log.protein_g);
    printf("Enter Carbohydrates (g): ");
    scanf("%f", &new_log.carbs_g);
    printf("Enter Fat (g): ");
    scanf("%f", &new_log.fat_g);

    FILE *file = fopen(LOG_FILE, "ab");
    if (file == NULL) {
        printf("Error: Could not open log file for writing.\n");
        press_enter_to_continue();
        return;
    }
    fwrite(&new_log, sizeof(MealLog), 1, file);
    fclose(file);

    printf("\nMeal logged successfully!\n");
    press_enter_to_continue();
}

float calculate_bmi(float weight_kg, float height_cm) {
    float height_m = cm_to_meters(height_cm);
    if (height_m <= 0) return 0.0;
    return weight_kg / (height_m * height_m);
}

void display_bmi_category(float bmi) {
    if (bmi <= 0) {
        printf("BMI: N/A\n");
        return;
    }
    printf("BMI: %.2f (", bmi);
    if (bmi < 18.5) printf("Underweight");
    else if (bmi < 24.9) printf("Normal weight");
    else if (bmi < 29.9) printf("Overweight");
    else printf("Obesity");
    printf(")\n");
}

void display_summary() {
    if (!profile.is_profile_set) {
        printf("\nError: Please create your profile first.\n");
        press_enter_to_continue();
        return;
    }

    printf("\n======================================================\n");
    printf("               C-FIT TRACKER SUMMARY                  \n");
    printf("======================================================\n");
    printf("PROFILE: %s (%d years, %s)\n", profile.name, profile.age, profile.gender);
    printf("Current Weight: %.1f kg, Height: %.1f cm\n", profile.weight_kg, profile.height_cm);

    float bmi = calculate_bmi(profile.weight_kg, profile.height_cm);
    printf("BMI Check: ");
    display_bmi_category(bmi);

    printf("\n--- HEALTH GOALS ---\n");
    printf("Target Weight: %.1f kg (Difference: %.1f kg)\n", profile.target_weight_kg, profile.weight_kg - profile.target_weight_kg);
    printf("Daily Calorie Goal: %d kcal\n", profile.daily_calorie_goal);
    printf("======================================================\n");

    FILE *file = fopen(LOG_FILE, "rb");
    if (file == NULL) {
        printf("\nLOG HISTORY: No meal entries found.\n");
        press_enter_to_continue();
        return;
    }

    MealLog log;
    printf("\n--- MEAL & CALORIE LOG HISTORY ---\n");
    printf("| %-10s | %-15s | %-7s | %-7s | %-7s | %-7s |\n", "DATE", "MEAL", "CALORIES", "PROTEIN", "CARBS", "FAT");
    printf("------------------------------------------------------------------------\n");

    int total_calories_today = 0;
    char today[11];
    get_current_date(today);

    while (fread(&log, sizeof(MealLog), 1, file) == 1) {
        printf("| %-10s | %-15s | %-7d | %-7.1f | %-7.1f | %-7.1f |\n",
               log.date, log.meal_name, log.calories, log.protein_g, log.carbs_g, log.fat_g);
        if (strcmp(log.date, today) == 0) total_calories_today += log.calories;
    }

    printf("------------------------------------------------------------------------\n");
    fclose(file);

    printf("\n--- TODAY'S TRACKING (%s) ---\n", today);
    printf("Total Calories Consumed Today: %d kcal\n", total_calories_today);
    printf("Calorie Goal: %d kcal\n", profile.daily_calorie_goal);
    if (total_calories_today <= profile.daily_calorie_goal)
        printf("Status: On Track! You have %d kcal remaining.\n", profile.daily_calorie_goal - total_calories_today);
    else
        printf("Status: Goal Exceeded! You are over by %d kcal.\n", total_calories_today - profile.daily_calorie_goal);

    printf("======================================================\n");
    press_enter_to_continue();
}

void display_visualization() {
    if (!profile.is_profile_set) {
        printf("\nError: Please create your profile first.\n");
        press_enter_to_continue();
        return;
    }

    FILE *file = fopen(LOG_FILE, "rb");
    if (file == NULL) {
        printf("\nDATA VISUALIZATION: Not enough data. Log some meals first.\n");
        press_enter_to_continue();
        return;
    }

    int daily_calories[7] = {0};
    char dates[7][11];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    for (int i = 0; i < 7; i++) {
        struct tm tm_calc = *tm_now;
        tm_calc.tm_mday -= (6 - i);
        mktime(&tm_calc);
        strftime(dates[i], 11, "%Y-%m-%d", &tm_calc);
    }

    MealLog log;
    while (fread(&log, sizeof(MealLog), 1, file) == 1) {
        for (int i = 0; i < 7; i++) {
            if (strcmp(log.date, dates[i]) == 0) {
                daily_calories[i] += log.calories;
                break;
            }
        }
    }
    fclose(file);

    int max_cal = profile.daily_calorie_goal * 2;
    for (int i = 0; i < 7; i++) if (daily_calories[i] > max_cal) max_cal = daily_calories[i];
    if (max_cal == 0) max_cal = 1;

    printf("\n======================================================\n");
    printf("          CALORIES CONSUMED (LAST 7 DAYS)             \n");
    printf("======================================================\n");
    printf("Goal: %d kcal\n\n", profile.daily_calorie_goal);

    int chart_height = 10;
    for (int h = chart_height; h >= 0; h--) {
        printf(" %-4d |", (int)(max_cal * (float)h / chart_height));
        for (int d = 0; d < 7; d++) {
            int bar_height = (int)(((float)daily_calories[d] / max_cal) * chart_height);
            if (h <= bar_height) {
                if (daily_calories[d] > profile.daily_calorie_goal && h > (int)(((float)profile.daily_calorie_goal / max_cal) * chart_height))
                    printf(" X ");
                else
                    printf(" # ");
            } else
                printf("   ");
        }
        printf("\n");
    }

    printf("-------+------------------------------------------\n");
    printf(" DATE  |");
    for (int d = 0; d < 7; d++) printf(" %s ", dates[d] + 5);
    printf("\n\n");
    printf("X = Over Goal, # = On Track/Below Goal.\n");
    printf("======================================================\n");

    press_enter_to_continue();
}

void display_menu() {
    printf("\n\n****************************************\n");
    printf("* C-FIT TRACKER MENU            *\n");
    printf("****************************************\n");
    printf("1. Create/Update Profile & Goals\n");
    printf("2. Log New Meal & Calories\n");
    printf("3. View Summary (BMI, Goals, History)\n");
    printf("4. View 7-Day Calorie Chart\n");
    printf("5. Exit\n");
    printf("****************************************\n");
    printf("Enter your choice: ");
}

void set_console_color() {
    printf("Choose console color (maroon/blue/black/purple/yellow): ");
    char color[20];
    scanf("%s", color);
    for (int i = 0; color[i]; i++) color[i] = tolower(color[i]);
    if (strcmp(color, "maroon") == 0) system("color 4F");
    else if (strcmp(color, "blue") == 0) system("color 1F");
    else if (strcmp(color, "black") == 0) system("color 0F");
    else if (strcmp(color, "purple") == 0) system("color 5F");
    else if (strcmp(color, "yellow") == 0) system("color 6F");
    else system("color 0F");
}

int main() {
    set_console_color();
    load_profile();

    int choice;
    do {
        system("cls || clear");
        display_menu();

        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input. Please enter a number.\n");
            int c; while ((c = getchar()) != '\n' && c != EOF);
            choice = 0;
            press_enter_to_continue();
            continue;
        }

        switch (choice) {
            case 1: create_or_update_profile(profile.is_profile_set); break;
            case 2: log_meal(); break;
            case 3: display_summary(); break;
            case 4: display_visualization(); break;
            case 5: printf("\nThank you for using C-Fit Tracker! Goodbye.\n"); break;
            default: printf("\nInvalid choice. Please select an option from 1 to 5.\n"); press_enter_to_continue();
        }

    } while (choice != 5);

    return 0;
}
