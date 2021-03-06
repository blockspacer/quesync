import React, { Component } from "react";

import { connect } from "react-redux";

import { Typography } from "@rmwc/typography";
import { TextField } from "@rmwc/textfield";
import { Button } from "@rmwc/button";

import { register, startAuth, finishAuth } from "../actions/userActions";

import zxcvbn from "zxcvbn";

import updater from "../updater";
import PasswordMeter from "./password_meter";

// Should be in rem units
const height = 35;
const width = 25;
const formClass = "quesync-register-form";

class RegisterForm extends Component {
	state = {
		password: "",
		usernameError: false,
		passwordError: false,
		passwordMismatchError: false,
		emailError: false
	};

	formatError = error => {
		if (this.state.usernameError) {
			return "Username field is missing or invalid!";
		} else if (this.state.emailError) {
			return "Email field is missing or invalid!";
		} else if (this.state.passwordError) {
			return "Password is too weak!";
		} else if (this.state.passwordMismatchError) {
			return "Passwords do not match!";
		}

		// Set the error message by the error code
		switch (error) {
			case 0:
				return "";

			case window.errors.invalid_username:
				return "The username entered is invalid!";

			case window.errors.invalid_email:
				return "The e-mail entered is invalid!";

			case window.errors.username_already_in_use:
				return "The username chosen is already taken!";

			case window.errors.email_already_in_use:
				return "The e-mail entered is already used!";

			default:
				return "Unknown error occurred!\nPlease try again later.";
		}
	};

	registerBtnClicked = event => {
		var username = this.refs.form[0].value,
			email = this.refs.form[1].value,
			password = this.refs.form[2].value,
			passwordVerification = this.refs.form[3].value;

		// Prevent the default load of the form
		event.preventDefault();

		// Blur all elements to prevent further input
		if (document && document.activeElement) document.activeElement.blur();

		// Reset errors
		this.setState({
			usernameError: false,
			passwordError: false,
			emailError: false,
			passwordMismatchError: false
		});

		// If the username field is empty
		if (
			username.length === 0 ||
			!/^[a-zA-Z0-9]+([_ -]?[a-zA-Z0-9]){3,20}$/.test(username)
		) {
			this.setState({
				usernameError: true
			});
			return;
		}
		// If the e-mail field is empty
		else if (
			email.length === 0 ||
			!/^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$/.test(email)
		) {
			this.setState({
				emailError: true
			});
			return;
		}
		// If the password field is empty
		else if (password.length === 0 || zxcvbn(password).score < 2) {
			this.setState({
				passwordError: true
			});
			return;
		}
		// If the password verification field doesn't match the password
		else if (password !== passwordVerification) {
			this.setState({
				passwordMismatchError: true
			});
			return;
		}

		// Start authenticating
		this.props.dispatch(startAuth());

		// Start the loading animation
		this.props.startLoadingAnimation(() => {
			this.props
				.dispatch(register(username, email, password))
				.then(async res => {
					const sessionId = res.value.sessionId;

					// Set the session id
					localStorage.setItem("_qpsid", sessionId);

					// Stop authenticating
					this.props.dispatch(finishAuth());

					// Fetch for the first time the data of the user
					await updater.update();

					// Transition to the app
					this.props.transitionToApp();
				})
				.catch(() => {
					// Stop the loading animation
					this.props.stopLoadingAnimation(() => {
						// Stop authenticating
						this.props.dispatch(finishAuth());
					});
				});
		});
	};

	static get height() {
		return height;
	}

	static get width() {
		return width;
	}

	static get formClass() {
		return formClass;
	}

	haveAccountBtnClicked = () => {
		this.props.transitionToLogin();
	};

	render() {
		return (
			<form
				className={"quesync-form " + formClass}
				ref="form"
				onSubmit={this.registerBtnClicked}
				style={{
					opacity: "0",
					pointerEvents: this.props.interactable ? "" : "none"
				}}
			>
				<Typography
					use="headline2"
					style={{
						color: "var(--mdc-theme-primary)",
						userSelect: "none"
					}}
				>
					Register
				</Typography>
				<div className="quesync-error-holder">
					<Typography
						className="quesync-register-error"
						use="caption"
						style={{
							color: "#ff1744",
							userSelect: "none",
							whiteSpace: "pre-line",
							lineHeight: "12px"
						}}
					>
						{this.formatError(this.props.error)}
					</Typography>
				</div>
				<TextField
					invalid={this.state.usernameError}
					outlined
					label="Username"
					icon="person"
					style={{ width: "300px" }}
				/>
				<TextField
					invalid={this.state.emailError}
					outlined
					label="E-mail"
					icon="email"
					style={{ marginTop: "15px", width: "300px" }}
				/>
				<TextField
					invalid={this.state.passwordError}
					outlined
					label="Password"
					type="password"
					icon="lock"
					onChange={evt =>
						this.setState({
							password: evt.target.value
						})
					}
					style={{ marginTop: "15px", width: "300px" }}
				/>
				<PasswordMeter password={this.state.password} />
				<TextField
					invalid={this.state.passwordMismatchError}
					outlined
					label="Re-enter password"
					type="password"
					icon="lock"
					style={{ marginTop: "15px", width: "300px" }}
				/>
				<Button
					type="submit"
					raised
					style={{ marginTop: "25px", width: "300px" }}
					theme={["secondary"]}
				>
					Register
				</Button>
				<Button
					type="button"
					raised
					style={{
						marginTop: "15px",
						width: "300px",
						background: "#ff6d00"
					}}
					onClick={this.haveAccountBtnClicked}
				>
					Already have an account?
				</Button>
			</form>
		);
	}
}

export default connect(state => ({
	error: state.user.authError
}))(RegisterForm);
