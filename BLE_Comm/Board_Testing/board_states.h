class BoardState {
  public:
    BoardState();
    bool sendMsg();
  private:
    State state;
};



class FSMMan {
public:  
  enum State {
      Default,
      IMUStreaming
    };
    
    State getState() { return m_state; }
    void setState() { } 
    void run()
    {
        switch(m_state)
        {
          case Default:
            break;
          case IMUStreaming:
            // build IMU message
            // send IMU message
              
        }
        
    }

private:

      State m_state;

};

void fsm()
{
  
}
