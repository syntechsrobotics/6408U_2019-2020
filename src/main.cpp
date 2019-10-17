#include "main.h"

#pragma region "Definitions"

//Controller(s?)
pros::Controller puppeteer(pros::E_CONTROLLER_MASTER);

//Drive motors
pros::Motor drive_FR(0, pros::E_MOTOR_GEARSET_18, true, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor drive_FL(1, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor drive_BR(2, pros::E_MOTOR_GEARSET_18, true, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor drive_BL(3, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);

//DR4B motors
pros::Motor DR4B_L(4, pros::E_MOTOR_GEARSET_36, false, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor DR4B_R(5, pros::E_MOTOR_GEARSET_36, true, pros::E_MOTOR_ENCODER_DEGREES);

//Claw - TODO: Change for 36:1 gearset
pros::Motor claw(6, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);
bool claw_open = false;
const float CLAW_MAX_ROTATION = 45;
//Spool - TODO: Change for 36:1 gearset
pros::Motor spool(7, pros::E_MOTOR_GEARSET_18, false, pros::E_MOTOR_ENCODER_DEGREES);

//Helper functions
int limitAbs(int n, int max)
{
	int sign;
	if (n > 0)
	{
		sign == 1;
	}
	else if (n < 0)
	{
		sign == -1;
	}
	else
	{
		return 0;
	}
	return std::min(max, abs(n)) * sign;
}
#pragma endregion
/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */

void initialize()
{
	//All drive motors coast - reduce jerk
	drive_FR.set_brake_mode(MOTOR_BRAKE_COAST);
	drive_FL.set_brake_mode(MOTOR_BRAKE_COAST);
	drive_BR.set_brake_mode(MOTOR_BRAKE_COAST);
	drive_BL.set_brake_mode(MOTOR_BRAKE_COAST);

	//DR4B set to hold - more stability
	DR4B_L.set_brake_mode(MOTOR_BRAKE_HOLD);
	DR4B_R.set_brake_mode(MOTOR_BRAKE_HOLD);

	//Claw set to hold - tighter grip on cubes
	claw.set_brake_mode(MOTOR_BRAKE_HOLD);

	//Spool set to hold - more constant pressure
	spool.set_brake_mode(MOTOR_BRAKE_HOLD);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol()
{
	int leftPower = 0;
	int rightPower = 0;
	double DR4BOffset = 0;
	while (true)
	{
		//Claw: toggle w/ A button
		if (puppeteer.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A))
		{
			claw_open = !claw_open;
		}

		if (claw_open)
		{
			claw.move_absolute(CLAW_MAX_ROTATION, 100);
		}
		else
		{
			claw.move_absolute(-5, 127);
		}

		//Spool: constant vel if claw closed
		if (!claw_open)
		{
			spool.move_velocity(100);
		}

		//DR4B: move w/ up/down directional buttons
		//If the two sides become offset, the side that is ahead slows down to compensate.
		DR4BOffset = DR4B_L.get_position() - DR4B_R.get_position();
		if (puppeteer.get_digital(pros::E_CONTROLLER_DIGITAL_UP))
		{
			DR4B_L.move(std::min((127 - DR4BOffset), 127));
			DR4B_R.move(std::min((127 + DR4BOffset), 127));
		}
		else if (puppeteer.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN))
		{
			DR4B_L.move(std::max((-127 + DR4BOffset), -127));
			DR4B_R.move(std::max((-127 - DR4BOffset), -127));
		}
		else
		{
			DR4B_L.move(0);
			DR4B_R.move(0);
		}

		//Drive: Arcade drive split on two sticks: forward/back on left, turning on right
		leftPower = limitAbs(puppeteer.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) + puppeteer.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X), 127);
		rightPower = limitAbs(puppeteer.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) - puppeteer.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X), 127);

		drive_FR.move(rightPower);
		drive_BR.move(rightPower);

		drive_FL.move(leftPower);
		drive_BL.move(leftPower);

		pros::delay(20);
	}
}
