import React, { Component } from 'react';
import './App.css';
import { TextField } from '@rmwc/textfield';
import { Button, ButtonIcon } from '@rmwc/button';
import { ThemeProvider } from '@rmwc/theme';
import { CircularProgress } from '@rmwc/circular-progress';
import 'material-components-web/dist/material-components-web.css'
import '@rmwc/circular-progress/circular-progress.css'

class App extends Component {
  constructor(props)
  {
    // Initialize the component super-class
    super(props);

    // Set initial state of component
    this.state = { 
      connecting: false
    }
  } 

  render() {
    return (
      <ThemeProvider className="App" options={{ primary: "blue" }} style={{ width: "100vw", height: "100vh", display: "flex", justifyContent: "center", alignItems: "center" }}>
        <div className="login-div" style={{ width: "350px", display: "flex", flexDirection: "column" }} >
          <TextField outlined label="E-Mail" />
          <TextField outlined label="Password" type="password" style={{ marginTop: "18px" }} />
          <TextField outlined label="Server IP" style={{ marginTop: "18px" }} />
          <div style={{ display: "flex", flexDirection: "row", justifyContent: "space-between" }}>
            <Button raised style={{ marginTop: "18px", width: "160px" }}>
              {
                this.state.connecting ? <ButtonIcon icon={<CircularProgress theme="secondary" />}/> : null
              }
              Login
            </Button>
            <Button raised style={{ marginTop: "18px", width: "160px" }}>Register</Button>
          </div>
        </div>
      </ThemeProvider>
    );
  }
}

export default App;
